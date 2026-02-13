#!/usr/bin/env python
'''
Compare rendered images between baseline and optimized MaterialX test runs
using NVIDIA FLIP perceptual image comparison.

FLIP (A Difference Evaluator for Alternating Images) approximates human
perception of differences when flipping between images. A FLIP score of 0
means identical, 1 means maximally different.

Usage:
    python diff_images.py <baseline_dir> <optimized_dir>
    python diff_images.py <baseline_dir> <optimized_dir> --threshold 0.05
    python diff_images.py <baseline_dir> <optimized_dir> --report report.html

Dependencies:
    pip install flip-evaluator Pillow numpy
'''

import argparse
import logging
import sys
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_images')

# Optional: FLIP (for perceptual image comparison)
_have_flip = False
try:
    import flip_evaluator as flip
    _have_flip = True
except ImportError:
    logger.debug('flip-evaluator not found. Install with: pip install flip-evaluator')


# =============================================================================
# IMAGE COMPARISON
# =============================================================================

def findImages(directory, pattern='**/*.png'):
    '''Find all PNG images in a directory recursively.'''
    directory = Path(directory)
    if not directory.exists():
        raise FileNotFoundError(f'Directory not found: {directory}')
    return list(directory.glob(pattern))


def computeImageDiff(img1Path, img2Path, ppd=70.0, heatmapPath=None):
    '''
    Compute FLIP perceptual difference metrics between two images.

    FLIP (A Difference Evaluator for Alternating Images) is a perceptual
    image comparison metric from NVIDIA that approximates human perception
    of differences when flipping between images.

    Args:
        img1Path: Path to reference (baseline) image
        img2Path: Path to test (optimized) image
        ppd: Pixels per degree (viewing distance). Default 70 assumes
             a 0.7m viewing distance for a 1080p 24" monitor.
        heatmapPath: Optional path to save FLIP heatmap image (magma colormap)

    Returns:
        dict with keys: mean_flip, max_flip, pct_diff_pixels, identical, heatmap_path
    '''
    import numpy as np

    try:
        flipMap, meanFlip, _ = flip.evaluate(
            str(img1Path),
            str(img2Path),
            "LDR",
            inputsRGB=True,
            applyMagma=False,
            computeMeanError=True,
            parameters={"ppd": ppd}
        )
    except Exception as e:
        return {
            'error': str(e),
            'identical': False
        }

    flipMap = np.array(flipMap)
    maxFlip = float(flipMap.max())

    # Percentage of pixels with perceptible difference (FLIP > 0.01)
    diffPixels = flipMap > 0.01
    pctDiffPixels = 100.0 * diffPixels.sum() / diffPixels.size

    result = {
        'mean_flip': float(meanFlip),
        'max_flip': maxFlip,
        'pct_diff_pixels': pctDiffPixels,
        'identical': meanFlip < 1e-6,
        'heatmap_path': None
    }

    # Save heatmap if requested
    if heatmapPath:
        try:
            heatmapImg, _, _ = flip.evaluate(
                str(img1Path),
                str(img2Path),
                "LDR",
                inputsRGB=True,
                applyMagma=True,
                computeMeanError=False,
                parameters={"ppd": ppd}
            )
            from PIL import Image
            heatmapArr = np.array(heatmapImg)
            if heatmapArr.max() <= 1.0:
                heatmapArr = (heatmapArr * 255).astype(np.uint8)
            Image.fromarray(heatmapArr).save(heatmapPath)
            result['heatmap_path'] = str(heatmapPath)
        except Exception as e:
            logger.warning(f'Failed to save heatmap: {e}')

    return result


def compareImages(baselineDir, optimizedDir, threshold=0.05, reportDir=None):
    '''
    Compare all matching images between two directories using FLIP.

    Args:
        baselineDir: Path to baseline images
        optimizedDir: Path to optimized images
        threshold: FLIP threshold above which to report differences (default: 0.05)
        reportDir: Optional directory to save FLIP heatmaps for HTML report

    Returns:
        List of comparison results with paths for report generation
    '''
    if not _have_flip:
        logger.error('Cannot compare images: flip-evaluator not installed.')
        logger.error('Install with: pip install flip-evaluator')
        return None

    baselineDir = Path(baselineDir)
    optimizedDir = Path(optimizedDir)

    # Create heatmap directory if generating report
    heatmapDir = None
    if reportDir:
        heatmapDir = Path(reportDir) / 'heatmaps'
        heatmapDir.mkdir(parents=True, exist_ok=True)

    baselineImages = findImages(baselineDir)
    logger.info(f'Found {len(baselineImages)} images in baseline')

    results = []
    matched = 0
    missing = 0

    for baselineImg in baselineImages:
        relPath = baselineImg.relative_to(baselineDir)
        optimizedImg = optimizedDir / relPath

        if not optimizedImg.exists():
            logger.warning(f'Missing in optimized: {relPath}')
            missing += 1
            continue

        matched += 1

        heatmapPath = None
        if heatmapDir:
            heatmapPath = heatmapDir / f'{relPath.stem}_flip.png'

        metrics = computeImageDiff(baselineImg, optimizedImg, heatmapPath=heatmapPath)
        metrics['name'] = relPath.stem
        metrics['path'] = str(relPath)
        metrics['baseline_path'] = str(baselineImg.absolute())
        metrics['optimized_path'] = str(optimizedImg.absolute())
        results.append(metrics)

    logger.info(f'Compared {matched} image pairs, {missing} missing')
    return results


def printImageTable(results, threshold=0.05):
    '''Print a formatted FLIP image comparison table to stdout.'''
    if results is None:
        return False

    print('\n' + '=' * 85)
    print(f"{'Image':<40} {'Mean FLIP':>10} {'Max FLIP':>10} {'% Diff':>10} {'Status':>8}")
    print('=' * 85)

    identical = 0
    different = 0
    errors = 0

    sortedResults = sorted(results, key=lambda x: x.get('mean_flip', 0), reverse=True)

    for r in sortedResults:
        name = r['name'][:38]

        if 'error' in r:
            print(f"{name:<40} {'ERROR':>10} {r['error']}")
            errors += 1
            continue

        meanFlip = r['mean_flip']
        maxFlip = r['max_flip']
        pctDiff = r['pct_diff_pixels']

        if r['identical'] or meanFlip < threshold:
            status = 'OK'
            identical += 1
        else:
            status = 'DIFF'
            different += 1

        print(f"{name:<40} {meanFlip:>10.6f} {maxFlip:>10.4f} {pctDiff:>9.2f}% {status:>8}")

    print('=' * 85)
    print(f'\nImage Summary (FLIP): {identical} identical, {different} different, {errors} errors')
    print(f'Threshold: mean FLIP < {threshold}')

    if different > 0:
        print(f'\n*** WARNING: {different} images differ above threshold! ***')
        return False
    else:
        print('\nAll images match within threshold.')
        return True


# =============================================================================
# HTML REPORT
# =============================================================================

def generateHtmlReport(reportPath, imageResults, threshold=0.05):
    '''
    Generate an HTML report with side-by-side image comparisons and FLIP heatmaps.

    Args:
        reportPath: Path to output HTML file
        imageResults: Image comparison results from compareImages()
        threshold: FLIP threshold used for pass/fail
    '''
    reportPath = Path(reportPath)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)

    def relPath(absPath):
        if absPath is None:
            return None
        try:
            return str(Path(absPath).relative_to(reportDir))
        except ValueError:
            return 'file:///' + str(Path(absPath)).replace('\\', '/')

    imgPassed = 0
    imgFailed = 0
    imgErrors = 0
    if imageResults:
        for r in imageResults:
            if 'error' in r:
                imgErrors += 1
            elif r['identical'] or r['mean_flip'] < threshold:
                imgPassed += 1
            else:
                imgFailed += 1

    html = []
    html.append('''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MaterialX Image Comparison Report</title>
    <style>
        * { box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 0; padding: 20px; background: #f5f5f5;
        }
        .container { max-width: 1800px; margin: 0 auto; }
        h1, h2 { color: #333; }
        h1 { border-bottom: 2px solid #3498db; padding-bottom: 10px; }

        .summary { display: flex; gap: 20px; margin-bottom: 30px; flex-wrap: wrap; }
        .card {
            background: white; border-radius: 8px; padding: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1); flex: 1; min-width: 200px;
        }
        .card h3 { margin: 0 0 10px 0; color: #666; font-size: 14px; text-transform: uppercase; }
        .card .value { font-size: 32px; font-weight: bold; }
        .card .value.good { color: #27ae60; }
        .card .value.bad { color: #e74c3c; }
        .card .value.neutral { color: #3498db; }

        .image-grid { display: grid; gap: 20px; }
        .image-row {
            display: grid; grid-template-columns: 1fr 1fr 1fr;
            gap: 10px; background: white; border-radius: 8px; padding: 15px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .image-row.failed { border-left: 4px solid #e74c3c; }
        .image-row.passed { border-left: 4px solid #27ae60; }
        .image-cell { text-align: center; }
        .image-cell img { max-width: 100%; height: auto; border: 1px solid #ddd; border-radius: 4px; }
        .image-cell .label { font-size: 12px; color: #666; margin-top: 5px; }
        .image-header {
            grid-column: 1 / -1; display: flex; justify-content: space-between;
            align-items: center; padding-bottom: 10px; border-bottom: 1px solid #eee;
        }
        .image-header h3 { margin: 0; }
        .image-header .metrics { font-size: 14px; color: #666; }
        .status-badge {
            display: inline-block; padding: 4px 12px; border-radius: 20px;
            font-size: 12px; font-weight: bold;
        }
        .status-badge.pass { background: #d4edda; color: #155724; }
        .status-badge.fail { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
<div class="container">
    <h1>MaterialX Image Comparison Report</h1>
''')

    # Summary cards
    html.append(f'''
    <div class="summary">
        <div class="card">
            <h3>Images Passed</h3>
            <div class="value good">{imgPassed}</div>
        </div>
        <div class="card">
            <h3>Images Failed</h3>
            <div class="value {'bad' if imgFailed > 0 else 'neutral'}">{imgFailed}</div>
        </div>
        <div class="card">
            <h3>Errors</h3>
            <div class="value {'bad' if imgErrors > 0 else 'neutral'}">{imgErrors}</div>
        </div>
        <div class="card">
            <h3>FLIP Threshold</h3>
            <div class="value neutral">{threshold}</div>
        </div>
    </div>
''')

    # Image comparisons
    if imageResults:
        html.append(f'''
    <h2>Image Comparisons (FLIP)</h2>
    <p>FLIP score: 0 = identical, 1 = maximally different. Threshold: {threshold}</p>
    <div class="image-grid">
''')
        sortedImages = sorted(imageResults, key=lambda x: x.get('mean_flip', 0), reverse=True)

        for r in sortedImages:
            if 'error' in r:
                continue

            passed = r['identical'] or r['mean_flip'] < threshold
            statusClass = 'passed' if passed else 'failed'
            statusBadge = 'pass' if passed else 'fail'
            statusText = 'PASS' if passed else 'FAIL'

            baselineRel = relPath(r.get('baseline_path'))
            optimizedRel = relPath(r.get('optimized_path'))
            heatmapRel = relPath(r.get('heatmap_path'))

            html.append(f'''
        <div class="image-row {statusClass}">
            <div class="image-header">
                <h3>{r['name']}</h3>
                <div class="metrics">
                    Mean FLIP: {r['mean_flip']:.4f} | Max: {r['max_flip']:.4f} | {r['pct_diff_pixels']:.1f}% pixels differ
                    <span class="status-badge {statusBadge}">{statusText}</span>
                </div>
            </div>
            <div class="image-cell">
                <img src="{baselineRel}" alt="Baseline">
                <div class="label">Baseline</div>
            </div>
            <div class="image-cell">
                <img src="{optimizedRel}" alt="Optimized">
                <div class="label">Optimized</div>
            </div>
            <div class="image-cell">
''')
            if heatmapRel:
                html.append(f'''                <img src="{heatmapRel}" alt="FLIP Heatmap">
                <div class="label">FLIP Heatmap</div>
''')
            else:
                html.append('''                <div style="padding: 50px; color: #999;">No heatmap</div>
''')
            html.append('''            </div>
        </div>
''')

        html.append('    </div>\n')

    # Footer
    html.append('''
    <footer style="margin-top: 40px; padding-top: 20px; border-top: 1px solid #ddd; color: #666; font-size: 12px;">
        Generated by diff_images.py | NVIDIA FLIP for perceptual image comparison
    </footer>
</div>
</body>
</html>
''')

    with open(reportPath, 'w', encoding='utf-8') as f:
        f.write(''.join(html))

    logger.info(f'HTML report saved to: {reportPath}')


# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Compare rendered images between baseline and optimized MaterialX test runs using NVIDIA FLIP.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s ./baseline/ ./optimized/
  %(prog)s ./baseline/ ./optimized/ --threshold 0.01
  %(prog)s ./baseline/ ./optimized/ --report comparison.html
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline directory containing rendered images')
    parser.add_argument('optimized', type=Path,
                        help='Optimized directory containing rendered images')
    parser.add_argument('--threshold', type=float, default=0.05,
                        help='FLIP threshold for pass/fail (default: 0.05)')
    parser.add_argument('--report', type=Path, default=None,
                        help='Path for HTML report with side-by-side images and FLIP heatmaps')

    args = parser.parse_args()

    if not _have_flip:
        logger.error('flip-evaluator is required. Install with: pip install flip-evaluator')
        sys.exit(1)

    # Determine report directory for heatmaps
    reportDir = None
    if args.report:
        reportDir = args.report.parent

    try:
        results = compareImages(args.baseline, args.optimized,
                                args.threshold, reportDir=reportDir)
        allPassed = printImageTable(results, args.threshold)

        if args.report and results:
            generateHtmlReport(args.report, results, args.threshold)

        sys.exit(0 if allPassed else 1)

    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)
    except Exception as e:
        logger.error(f'Error: {e}')
        raise


if __name__ == '__main__':
    main()
