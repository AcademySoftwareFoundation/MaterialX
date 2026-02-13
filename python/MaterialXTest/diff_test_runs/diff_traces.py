#!/usr/bin/env python
'''
Compare performance traces between baseline and optimized MaterialX test runs.

Reads Perfetto .perfetto-trace files and compares slice durations, generating
tables, charts, and an HTML report.

Two modes:
  --gpu           Compare GPU render durations per material (from GPU async track)
  --slice NAME    Compare CPU slice durations per material (child slices under
                  material parent slices). Multiple names produce multiple charts.

For image comparison, see diff_images.py in the same directory.

Usage:
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu
    python diff_traces.py <baseline_dir> <optimized_dir> --slice GenerateShader
    python diff_traces.py <baseline_dir> <optimized_dir> --slice GenerateShader CompileShader
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu --slice GenerateShader
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu -o custom_name.html
'''

import argparse
import logging
import sys
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_traces')


# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

try:
    from perfetto.trace_processor import TraceProcessor
except ImportError:
    sys.exit('ERROR: perfetto is required. Install with: pip install perfetto')

try:
    import pandas as pd
except ImportError:
    sys.exit('ERROR: pandas is required. Install with: pip install pandas')

# Import shared reporting utilities (works both as package and standalone script)
try:
    from ._report import (mergeComparisonDf, printComparisonTable,
                           createComparisonChart, generateHtmlReport,
                           chartPath, openReport)
except ImportError:
    from _report import (mergeComparisonDf, printComparisonTable,
                          createComparisonChart, generateHtmlReport,
                          chartPath, openReport)


# =============================================================================
# TRACE LOADING
# =============================================================================

def findTraceFile(path):
    '''
    Find a Perfetto trace file from a path.

    If path is a file, return it directly.
    If path is a directory, search for *.perfetto-trace files.
    Returns the first trace file found, or raises FileNotFoundError.
    '''
    path = Path(path)

    if path.is_file():
        return path

    if path.is_dir():
        traces = list(path.glob('*.perfetto-trace'))
        if not traces:
            traces = list(path.glob('**/*.perfetto-trace'))
        if traces:
            if len(traces) > 1:
                print(f'Warning: Multiple traces found in {path}, using: {traces[0].name}')
            return traces[0]
        raise FileNotFoundError(f'No .perfetto-trace files found in: {path}')

    raise FileNotFoundError(f'Path not found: {path}')


def loadSliceDurations(traceProcessor, trackName=None):
    '''
    Load slice durations from a Perfetto trace, optionally filtered by track.

    Results are ordered by timestamp to preserve frame ordering within each
    material (important for warmup frame discarding).

    Args:
        traceProcessor: TraceProcessor instance
        trackName: Optional track name filter (e.g., "GPU")

    Returns:
        DataFrame with columns [name, dur_ms].
    '''

    if trackName:
        query = f'''
        SELECT slice.name, slice.dur / 1000000.0 as dur_ms
        FROM slice
        JOIN track ON slice.track_id = track.id
        WHERE track.name = '{trackName}'
        ORDER BY slice.ts
        '''
    else:
        query = '''
        SELECT slice.name, slice.dur / 1000000.0 as dur_ms
        FROM slice
        JOIN track ON slice.track_id = track.id
        ORDER BY slice.ts
        '''

    df = traceProcessor.query(query).as_pandas_dataframe()
    if df.empty:
        trackMsg = f' on track "{trackName}"' if trackName else ''
        logger.warning(f'No slices found{trackMsg}')
        return pd.DataFrame(columns=['name', 'dur_ms'])
    return df


def loadChildSliceDurations(traceProcessor, sliceName):
    '''
    Load durations of a named child slice, keyed by parent (material) name.

    Queries the trace for slices matching sliceName that are direct children
    of a parent slice (typically the material name).

    Args:
        traceProcessor: TraceProcessor instance
        sliceName: Name of the child slice (e.g., "GenerateShader", "CompileShader")

    Returns:
        DataFrame with columns [name, dur_ms]. 'name' is the parent (material) name.
    '''

    query = f'''
    SELECT parent.name as name, child.dur / 1000000.0 as dur_ms
    FROM slice child
    JOIN slice parent ON child.parent_id = parent.id
    WHERE child.name = '{sliceName}'
    ORDER BY parent.name
    '''

    df = traceProcessor.query(query).as_pandas_dataframe()
    if df.empty:
        logger.warning(f'No "{sliceName}" slices found')
        return pd.DataFrame(columns=['name', 'dur_ms'])
    return df


def loadOptimizationEvents(traceProcessor, optimizationName=None):
    '''
    Load optimization events from a Perfetto trace.

    Optimization events are nested inside ShaderGen slices with hierarchy:
    MaterialName -> GenerateShader -> OptimizationPass

    Args:
        traceProcessor: TraceProcessor instance
        optimizationName: Filter by optimization pass name

    Returns:
        Set of material names that had the optimization applied.
    '''

    if optimizationName:
        query = f'''
        SELECT DISTINCT grandparent.name as material_name
        FROM slice opt
        JOIN slice parent ON opt.parent_id = parent.id
        JOIN slice grandparent ON parent.parent_id = grandparent.id
        WHERE opt.name = "{optimizationName}"
        '''
    else:
        query = '''
        SELECT DISTINCT opt.name as opt_name, grandparent.name as material_name
        FROM slice opt
        JOIN slice parent ON opt.parent_id = parent.id
        JOIN slice grandparent ON parent.parent_id = grandparent.id
        '''

    result = traceProcessor.query(query)

    optimizedMaterials = set()
    for row in result:
        if row.material_name:
            optimizedMaterials.add(row.material_name)

    return optimizedMaterials


# =============================================================================
# COMPARISON
# =============================================================================

def _aggregateByName(df, warmupFrames=0):
    '''
    Aggregate durations by name (averaging across multiple samples).

    Args:
        df: DataFrame with columns [name, dur_ms], ordered by timestamp.
        warmupFrames: Number of initial frames per material to discard
                      before averaging (burn-in period).
    '''
    if df.empty:
        return pd.DataFrame(columns=['name', 'value'])

    if warmupFrames > 0:
        # Within each material group, drop the first N rows (preserving
        # chronological order from the ORDER BY slice.ts query).
        df = (df.groupby('name', sort=False)
              .apply(lambda g: g.iloc[warmupFrames:] if len(g) > warmupFrames
                     else g.iloc[0:0], include_groups=False)
              .reset_index(level=0))
        if df.empty:
            logger.warning(f'All samples discarded by warmup ({warmupFrames} frames)')
            return pd.DataFrame(columns=['name', 'value'])

    agg = df.groupby('name')['dur_ms'].mean().reset_index()
    agg.columns = ['name', 'value']
    return agg


def compareGpuTraces(baselineTraceProcessor, optimizedTraceProcessor,
                     minDeltaMs=0.0, warmupFrames=0):
    '''
    Compare GPU render durations between baseline and optimized traces.

    Reads the GPU async track and averages per material across frames,
    optionally discarding initial warmup frames.

    Returns:
        (merged_df, totalFrames, usedFrames) -- the comparison DataFrame,
        the total number of frames per material, and how many were used
        after warmup discarding.
    '''
    logger.info('Comparing GPU traces...')
    baselineData = loadSliceDurations(baselineTraceProcessor, trackName='GPU')
    optimizedData = loadSliceDurations(optimizedTraceProcessor, trackName='GPU')

    # Determine typical samples per material (frames rendered)
    if not baselineData.empty:
        totalFrames = int(baselineData.groupby('name').size().median())
    else:
        totalFrames = 0

    usedFrames = max(0, totalFrames - warmupFrames)

    baselineAgg = _aggregateByName(baselineData, warmupFrames=warmupFrames)
    optimizedAgg = _aggregateByName(optimizedData, warmupFrames=warmupFrames)

    return (mergeComparisonDf(baselineAgg, optimizedAgg, minDelta=minDeltaMs),
            totalFrames, usedFrames)


def compareChildSlices(baselineTraceProcessor, optimizedTraceProcessor, sliceName, minDeltaMs=0.0):
    '''
    Compare durations of a named child slice (e.g., GenerateShader) per material.

    Queries child slices under material parent slices in both traces,
    then merges by material name and computes delta/percentage.
    '''
    logger.info(f'Comparing "{sliceName}" slices...')
    baselineData = loadChildSliceDurations(baselineTraceProcessor, sliceName)
    baselineAgg = _aggregateByName(baselineData)

    optimizedData = loadChildSliceDurations(optimizedTraceProcessor, sliceName)
    optimizedAgg = _aggregateByName(optimizedData)

    return mergeComparisonDf(baselineAgg, optimizedAgg, minDelta=minDeltaMs)


# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Compare performance traces between baseline and optimized MaterialX test runs.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s ./baseline/ ./optimized/ --gpu
  %(prog)s ./baseline/ ./optimized/ --slice GenerateShader
  %(prog)s ./baseline/ ./optimized/ --slice GenerateShader CompileShader RenderMaterial
  %(prog)s ./baseline/ ./optimized/ --gpu --slice GenerateShader
  %(prog)s ./baseline/ ./optimized/ --gpu -o custom_name.html

For image comparison, see diff_images.py in the same directory.
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline directory containing Perfetto traces')
    parser.add_argument('optimized', type=Path,
                        help='Optimized directory containing Perfetto traces')

    modeGroup = parser.add_argument_group('comparison modes (at least one required)')
    modeGroup.add_argument('--gpu', action='store_true',
                           help='Compare GPU render durations per material')
    modeGroup.add_argument('--slice', nargs='+', metavar='NAME',
                           help='Compare named child-slice durations per material '
                                '(e.g., GenerateShader, CompileShader, RenderMaterial)')

    optGroup = parser.add_argument_group('options')
    optGroup.add_argument('--min-delta-ms', type=float, default=0.0,
                          help='Minimum absolute time difference in ms to include')
    optGroup.add_argument('-o', '--outputfile', dest='outputfile', type=str,
                          default=None,
                          help='Output HTML report file name (default: <baseline>_vs_<optimized>.html)')
    optGroup.add_argument('--warmup-frames', type=int, default=0,
                          help='Number of initial GPU frames per material to discard '
                               'as burn-in before averaging (default: 0)')
    optGroup.add_argument('--show-opt', type=str, metavar='OPT_NAME',
                          help='Highlight materials affected by optimization pass')

    args = parser.parse_args()

    if not args.gpu and not args.slice:
        parser.print_help()
        sys.exit(0)

    # Load trace files once
    try:
        baselineTracePath = findTraceFile(args.baseline)
        optimizedTracePath = findTraceFile(args.optimized)
    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)

    logger.info(f'Loading baseline trace: {baselineTracePath}')
    baselineTraceProcessor = TraceProcessor(trace=str(baselineTracePath))

    logger.info(f'Loading optimized trace: {optimizedTracePath}')
    optimizedTraceProcessor = TraceProcessor(trace=str(optimizedTracePath))

    # Load optimization events if requested
    optimizedMaterials = set()
    if args.show_opt:
        baselineMaterials = loadOptimizationEvents(baselineTraceProcessor, args.show_opt)
        if baselineMaterials:
            logger.error(f'ERROR: Baseline has {len(baselineMaterials)} materials '
                        f'with {args.show_opt}!')
            sys.exit(1)

        optimizedMaterials = loadOptimizationEvents(optimizedTraceProcessor, args.show_opt)
        logger.info(f'Found {len(optimizedMaterials)} materials affected by {args.show_opt}')

    # Directory leaf names for display
    baselineName = Path(args.baseline).name
    optimizedName = Path(args.optimized).name

    # Derive default report name from directory names
    if args.outputfile is None:
        args.outputfile = f'{baselineName}_vs_{optimizedName}.html'

    # Build the list of comparisons to run
    comparisons = []

    if args.gpu:
        comparisons.append('GPU')

    if args.slice:
        for sliceName in args.slice:
            comparisons.append(sliceName)

    # Build filter subtitle from active options
    filterParts = []
    if args.min_delta_ms > 0:
        filterParts.append(f'min delta: {args.min_delta_ms:.1f} ms')
    if args.warmup_frames > 0:
        filterParts.append(f'warmup: {args.warmup_frames} frames discarded')
    if args.show_opt:
        filterParts.append(f'highlighting: {args.show_opt}')
    subtitle = 'Filters: ' + ', '.join(filterParts) if filterParts else None

    # Derive chart paths from the report file name
    reportPath = Path(args.outputfile)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)
    chartBase = reportDir / (reportPath.stem + '.svg')

    reportSections = []

    try:
        for label in comparisons:
            # Run comparison and build title
            if label == 'GPU':
                traceData, totalFrames, usedFrames = compareGpuTraces(
                    baselineTraceProcessor, optimizedTraceProcessor,
                    args.min_delta_ms, args.warmup_frames)
                if usedFrames > 1:
                    if args.warmup_frames > 0:
                        avgNote = (f' (averaged over {usedFrames} of {totalFrames} frames, '
                                   f'{args.warmup_frames} warmup discarded)')
                    else:
                        avgNote = f' (averaged over {totalFrames} frames)'
                else:
                    avgNote = ''
                title = f'GPU Render Duration per Material{avgNote}: {baselineName} vs {optimizedName}'
            else:
                traceData = compareChildSlices(
                    baselineTraceProcessor, optimizedTraceProcessor, label, args.min_delta_ms)
                title = f'{label} Duration per Material: {baselineName} vs {optimizedName}'

            # Print table
            printComparisonTable(traceData, title,
                                baselineLabel=baselineName,
                                optimizedLabel=optimizedName,
                                unit='ms',
                                highlightNames=optimizedMaterials)

            # Generate chart
            if traceData is not None and not traceData.empty:
                svgPath = chartPath(chartBase, label)
                createComparisonChart(
                    traceData, svgPath, title=title,
                    baselineLabel=baselineName, optimizedLabel=optimizedName,
                    unit='ms',
                    highlightNames=optimizedMaterials,
                    highlightLabel=f'affected by {args.show_opt}' if args.show_opt else None,
                    subtitle=subtitle)
                reportSections.append((title, svgPath))

        # HTML Report (always generated)
        pageTitle = f'Trace Comparison: {baselineName} vs {optimizedName}'
        if reportSections:
            generateHtmlReport(reportPath, reportSections, pageTitle=pageTitle,
                               subtitle=subtitle,
                               footerText='Generated by diff_traces.py')
            openReport(reportPath)
        else:
            print(f'\n{"=" * 85}')
            print('  No data to report.')
            print(f'{"=" * 85}')

        sys.exit(0)

    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)
    except Exception as e:
        logger.error(f'Error: {e}')
        raise


if __name__ == '__main__':
    main()
