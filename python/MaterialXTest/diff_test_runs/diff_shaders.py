#!/usr/bin/env python
'''
Compare dumped shader files between baseline and optimized MaterialX test runs.

Computes per-material metrics from generated GLSL shader source files and,
optionally, from external shader analysis tools found in PATH.

Built-in metrics (always available):
  LOC   Lines of code (non-blank lines in the pixel shader)

Tool-based metrics (when the tool is in PATH):
  glslangValidator  SPIR-V size (bytes) + compilation time (ms)
  spirv-opt         Optimised SPIR-V size + optimisation time (ms)

Usage:
    python diff_shaders.py <baseline_dir> <optimized_dir>
    python diff_shaders.py <baseline_dir> <optimized_dir> -o shader_diff.html
'''

import argparse
import logging
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_shaders')

# Import shared reporting utilities (works both as package and standalone script)
try:
    from ._report import (mergeComparison, printComparisonTable,
                           createComparisonChart, generateHtmlReport,
                           chartPath, openReport)
except ImportError:
    from _report import (mergeComparison, printComparisonTable,
                          createComparisonChart, generateHtmlReport,
                          chartPath, openReport)


# =============================================================================
# SHADER FILE DISCOVERY
# =============================================================================

def findShaderPairs(baselineDir, optimizedDir, pattern='**/*_ps.glsl'):
    '''
    Find matching pixel shader files between baseline and optimized directories.

    Args:
        baselineDir: Path to baseline shader directory
        optimizedDir: Path to optimized shader directory
        pattern: Glob pattern for shader files (default: pixel shaders)

    Returns:
        List of (materialName, baselinePath, optimizedPath) tuples,
        sorted by material name.
    '''
    baselineDir = Path(baselineDir)
    optimizedDir = Path(optimizedDir)

    pairs = []
    for baselineFile in sorted(baselineDir.glob(pattern)):
        relPath = baselineFile.relative_to(baselineDir)
        optimizedFile = optimizedDir / relPath

        if not optimizedFile.exists():
            logger.warning(f'Missing in optimized: {relPath}')
            continue

        # Derive material name from filename (strip _ps.glsl suffix)
        stem = baselineFile.stem
        if stem.endswith('_ps'):
            materialName = stem[:-3]
        elif stem.endswith('_vs'):
            materialName = stem[:-3]
        else:
            materialName = stem

        pairs.append((materialName, baselineFile, optimizedFile))

    logger.info(f'Found {len(pairs)} matching shader pairs')
    return pairs


# =============================================================================
# METRICS: LOC (always available)
# =============================================================================

def countLoc(shaderPath):
    '''Count non-blank lines in a shader file.'''
    text = Path(shaderPath).read_text(encoding='utf-8', errors='replace')
    return sum(1 for line in text.splitlines() if line.strip())


def computeLocMetrics(pairs):
    '''
    Compute LOC (lines of code) for all shader pairs.

    Returns:
        (baselineDict, optimizedDict) -- {materialName: loc} for each side.
    '''
    baseline = {}
    optimized = {}
    for materialName, baselinePath, optimizedPath in pairs:
        baseline[materialName] = countLoc(baselinePath)
        optimized[materialName] = countLoc(optimizedPath)
    return baseline, optimized


# =============================================================================
# SPIR-V COMPILATION PIPELINE  (glslangValidator, compile once / reuse)
# =============================================================================

def _compileToSpirvTimed(glslPath, outputPath):
    '''
    Compile a GLSL shader to SPIR-V using glslangValidator.

    Uses OpenGL semantics (-G) since MaterialX generates OpenGL GLSL,
    and --auto-map-locations to assign layout locations automatically
    (MaterialX shaders don't have explicit layout qualifiers).

    Returns (success: bool, elapsedMs: float).
    '''
    try:
        start = time.perf_counter()
        result = subprocess.run(
            ['glslangValidator', '-G', '--auto-map-locations',
             '-S', 'frag', '-o', str(outputPath), str(glslPath)],
            capture_output=True, text=True, timeout=30
        )
        elapsedMs = (time.perf_counter() - start) * 1000.0
        if result.returncode != 0:
            logger.debug(f'glslangValidator stderr: {result.stderr.strip()}')
        return result.returncode == 0, elapsedMs
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return False, 0.0


def compileSpirvPairs(pairs, tmpDir):
    '''
    Compile all shader pairs to SPIR-V once, caching results in tmpDir.

    Returns:
        dict of materialName -> {
            'baseline_spv': Path, 'optimized_spv': Path,
            'baseline_compile_ms': float, 'optimized_compile_ms': float,
        }
    '''
    cache = {}
    for materialName, baselinePath, optimizedPath in pairs:
        bSpv = tmpDir / f'{materialName}_baseline.spv'
        oSpv = tmpDir / f'{materialName}_optimized.spv'

        bOk, bMs = _compileToSpirvTimed(baselinePath, bSpv)
        oOk, oMs = _compileToSpirvTimed(optimizedPath, oSpv)

        if bOk and oOk:
            cache[materialName] = {
                'baseline_spv': bSpv,
                'optimized_spv': oSpv,
                'baseline_compile_ms': bMs,
                'optimized_compile_ms': oMs,
            }
        else:
            logger.warning(f'SPIR-V compilation failed for {materialName}')

    logger.info(f'Compiled {len(cache)}/{len(pairs)} shader pairs to SPIR-V')
    return cache


# =============================================================================
# SPIR-V OPTIMISATION PIPELINE  (spirv-opt, reuses compiled SPIR-V)
# =============================================================================

def _optimizeSpirvTimed(inputPath, outputPath):
    '''Run spirv-opt -O on a SPIR-V binary.  Returns (success, elapsedMs).'''
    try:
        start = time.perf_counter()
        result = subprocess.run(
            ['spirv-opt', '-O', '-o', str(outputPath), str(inputPath)],
            capture_output=True, text=True, timeout=60
        )
        elapsedMs = (time.perf_counter() - start) * 1000.0
        return result.returncode == 0, elapsedMs
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return False, 0.0


def optimizeSpirvPairs(spirvCache, tmpDir):
    '''
    Run spirv-opt on every cached SPIR-V pair.

    Returns:
        dict of materialName -> {
            'baseline_opt_spv': Path, 'optimized_opt_spv': Path,
            'baseline_opt_ms': float, 'optimized_opt_ms': float,
        }
    '''
    optCache = {}
    for name, info in spirvCache.items():
        bOpt = tmpDir / f'{name}_baseline_opt.spv'
        oOpt = tmpDir / f'{name}_optimized_opt.spv'

        bOk, bMs = _optimizeSpirvTimed(info['baseline_spv'], bOpt)
        oOk, oMs = _optimizeSpirvTimed(info['optimized_spv'], oOpt)

        if bOk and oOk:
            optCache[name] = {
                'baseline_opt_spv': bOpt,
                'optimized_opt_spv': oOpt,
                'baseline_opt_ms': bMs,
                'optimized_opt_ms': oMs,
            }
        else:
            logger.warning(f'spirv-opt failed for {name}')

    logger.info(f'Optimised {len(optCache)}/{len(spirvCache)} SPIR-V pairs')
    return optCache


# =============================================================================
# METRICS EXTRACTORS  (pull numbers out of the caches)
# =============================================================================

def extractSpirvSizeMetrics(spirvCache):
    '''SPIR-V binary size in bytes.'''
    b, o = {}, {}
    for name, info in spirvCache.items():
        b[name] = info['baseline_spv'].stat().st_size
        o[name] = info['optimized_spv'].stat().st_size
    return b, o


def extractCompileTimeMetrics(spirvCache):
    '''glslangValidator compilation time in ms.'''
    b, o = {}, {}
    for name, info in spirvCache.items():
        b[name] = info['baseline_compile_ms']
        o[name] = info['optimized_compile_ms']
    return b, o


def extractOptSpirvSizeMetrics(optCache):
    '''Optimised SPIR-V binary size in bytes.'''
    b, o = {}, {}
    for name, info in optCache.items():
        b[name] = info['baseline_opt_spv'].stat().st_size
        o[name] = info['optimized_opt_spv'].stat().st_size
    return b, o


def extractOptTimeMetrics(optCache):
    '''spirv-opt optimisation time in ms.'''
    b, o = {}, {}
    for name, info in optCache.items():
        b[name] = info['baseline_opt_ms']
        o[name] = info['optimized_opt_ms']
    return b, o


# =============================================================================
# REPORT HELPERS
# =============================================================================

def _addMetric(reportSections, data, metricTag, title, baselineName,
               optimizedName, chartBase, unit='', valueFormat='.0f'):
    '''
    Print a comparison table, generate a chart, and append to reportSections.
    '''
    if data is None or data.empty:
        return

    printComparisonTable(data, title,
                         baselineLabel=baselineName,
                         optimizedLabel=optimizedName,
                         unit=unit, valueFormat=valueFormat)

    svgPath = chartPath(chartBase, metricTag)
    createComparisonChart(data, svgPath, title=title,
                          baselineLabel=baselineName,
                          optimizedLabel=optimizedName,
                          unit=unit)
    reportSections.append((title, svgPath))


# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Compare dumped shader files between baseline and '
                    'optimized MaterialX test runs.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s ./baseline/ ./optimized/
  %(prog)s ./baseline/ ./optimized/ -o shader_diff.html
  %(prog)s ./baseline/ ./optimized/ --pattern "**/*_vs.glsl"

Available metrics depend on tools found in PATH:
  LOC               Always available (non-blank line count)
  SPIR-V size       Requires glslangValidator
  Compile time      Requires glslangValidator
  Optimised SPIR-V  Requires glslangValidator + spirv-opt
  spirv-opt time    Requires glslangValidator + spirv-opt
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline directory containing dumped shaders')
    parser.add_argument('optimized', type=Path,
                        help='Optimized directory containing dumped shaders')
    parser.add_argument('-o', '--outputfile', type=str, default=None,
                        help='Output HTML report file name '
                             '(default: <baseline>_vs_<optimized>_shaders.html)')
    parser.add_argument('--pattern', type=str, default='**/*_ps.glsl',
                        help='Glob pattern for shader files (default: **/*_ps.glsl)')

    args = parser.parse_args()

    # Discover shader pairs ------------------------------------------------
    try:
        pairs = findShaderPairs(args.baseline, args.optimized, args.pattern)
    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)

    if not pairs:
        logger.error('No matching shader pairs found.')
        sys.exit(1)

    # Directory leaf names for display
    baselineName = Path(args.baseline).name
    optimizedName = Path(args.optimized).name

    # Derive default report name
    if args.outputfile is None:
        args.outputfile = f'{baselineName}_vs_{optimizedName}_shaders.html'

    reportPath = Path(args.outputfile)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)
    chartBase = reportDir / (reportPath.stem + '.svg')

    reportSections = []

    # Discover available tools ---------------------------------------------
    tools = {
        'glslangValidator': shutil.which('glslangValidator'),
        'spirv-opt': shutil.which('spirv-opt'),
    }
    foundTools = [name for name, path in tools.items() if path]
    if foundTools:
        logger.info(f'Found tools in PATH: {", ".join(foundTools)}')
    else:
        logger.info('No optional shader tools found in PATH; '
                     'only LOC will be computed')

    # ---- Metric: LOC (always) --------------------------------------------
    logger.info('Computing LOC metrics...')
    bLoc, oLoc = computeLocMetrics(pairs)
    locData = mergeComparison(bLoc, oLoc)
    _addMetric(reportSections, locData, 'LOC',
               f'Lines of Code (non-blank): {baselineName} vs {optimizedName}',
               baselineName, optimizedName, chartBase, unit=' lines')

    # ---- Tool-based metrics (inside a temp directory) --------------------
    with tempfile.TemporaryDirectory(prefix='mtlx_spirv_') as tmpDir:
        tmpPath = Path(tmpDir)
        spirvCache = {}
        optCache = {}

        # -- Compile GLSL -> SPIR-V (once) ---------------------------------
        if tools['glslangValidator']:
            spirvCache = compileSpirvPairs(pairs, tmpPath)

            if spirvCache:
                # SPIR-V Size
                bSize, oSize = extractSpirvSizeMetrics(spirvCache)
                _addMetric(
                    reportSections, mergeComparison(bSize, oSize), 'SPIRV',
                    f'SPIR-V Size (bytes): {baselineName} vs {optimizedName}',
                    baselineName, optimizedName, chartBase, unit=' B')

                # Compilation Time
                bTime, oTime = extractCompileTimeMetrics(spirvCache)
                _addMetric(
                    reportSections, mergeComparison(bTime, oTime), 'compile_time',
                    f'glslangValidator Compile Time (ms): '
                    f'{baselineName} vs {optimizedName}',
                    baselineName, optimizedName, chartBase,
                    unit=' ms', valueFormat='.1f')

        # -- spirv-opt on cached SPIR-V ------------------------------------
        if tools['spirv-opt'] and spirvCache:
            optCache = optimizeSpirvPairs(spirvCache, tmpPath)

            if optCache:
                # Optimised SPIR-V Size
                bOptSize, oOptSize = extractOptSpirvSizeMetrics(optCache)
                _addMetric(
                    reportSections, mergeComparison(bOptSize, oOptSize),
                    'SPIRV_opt',
                    f'Optimised SPIR-V Size (bytes): '
                    f'{baselineName} vs {optimizedName}',
                    baselineName, optimizedName, chartBase, unit=' B')

                # spirv-opt Time
                bOptTime, oOptTime = extractOptTimeMetrics(optCache)
                _addMetric(
                    reportSections, mergeComparison(bOptTime, oOptTime),
                    'spirvopt_time',
                    f'spirv-opt Time (ms): {baselineName} vs {optimizedName}',
                    baselineName, optimizedName, chartBase,
                    unit=' ms', valueFormat='.1f')

    # ---- HTML Report -----------------------------------------------------
    pageTitle = f'Shader Comparison: {baselineName} vs {optimizedName}'
    toolInfo = ', '.join(foundTools) if foundTools else 'none'
    footerText = (f'Generated by diff_shaders.py &mdash; '
                  f'tools used: {toolInfo}')

    if reportSections:
        generateHtmlReport(reportPath, reportSections, pageTitle=pageTitle,
                           footerText=footerText)
        openReport(reportPath)
    else:
        print(f'\n{"=" * 85}')
        print('  No data to report.')
        print(f'{"=" * 85}')


if __name__ == '__main__':
    main()
