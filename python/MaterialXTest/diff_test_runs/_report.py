'''
Shared reporting utilities for MaterialX diff tools.

Provides comparison table formatting, chart generation (SVG via matplotlib),
and HTML report building for comparing per-material metrics between
baseline and optimized test runs.

Data convention:
    All comparison functions expect a pandas DataFrame with columns:
        name        -- item name (material, shader file, etc.)
        baseline    -- baseline metric value
        optimized   -- optimized metric value
        delta       -- optimized - baseline
        change_pct  -- (delta / baseline) * 100
'''

import logging
from pathlib import Path

import pandas as pd

logger = logging.getLogger(__name__)


# -----------------------------------------------------------------------------
# Optional: matplotlib (for chart generation)
# -----------------------------------------------------------------------------

_have_matplotlib = False
try:
    import matplotlib
    matplotlib.rcParams['svg.fonttype'] = 'none'  # Keep text as <text>, not paths
    import matplotlib.pyplot as plt
    from matplotlib.patches import Patch
    _have_matplotlib = True
except ImportError:
    pass


# =============================================================================
# DATA HELPERS
# =============================================================================

def isna(val):
    '''Check if value is None or NaN.'''
    return val is None or pd.isna(val)


def mergeComparison(baselineValues, optimizedValues, minDelta=0.0):
    '''
    Build a comparison DataFrame from two {name: value} dicts.

    Args:
        baselineValues: dict mapping name -> numeric value
        optimizedValues: dict mapping name -> numeric value
        minDelta: Minimum absolute delta to include (0 = include all)

    Returns:
        DataFrame with columns [name, baseline, optimized, delta, change_pct],
        sorted by delta ascending (biggest improvements first).
    '''
    baselineDf = pd.DataFrame(
        list(baselineValues.items()), columns=['name', 'baseline'])
    optimizedDf = pd.DataFrame(
        list(optimizedValues.items()), columns=['name', 'optimized'])

    merged = pd.merge(baselineDf, optimizedDf, on='name', how='outer')
    merged['delta'] = merged['optimized'] - merged['baseline']
    merged['change_pct'] = (merged['delta'] / merged['baseline']) * 100

    if minDelta > 0:
        merged = merged[merged['delta'].abs() >= minDelta]

    return merged.sort_values('delta', ascending=True).reset_index(drop=True)


def mergeComparisonDf(baselineAgg, optimizedAgg, minDelta=0.0):
    '''
    Build a comparison DataFrame by merging two aggregated DataFrames.

    Each input must have columns [name, value].  This is useful when
    values come from pandas groupby rather than plain dicts.

    Returns:
        DataFrame with columns [name, baseline, optimized, delta, change_pct],
        sorted by delta ascending.
    '''
    merged = pd.merge(
        baselineAgg[['name', 'value']],
        optimizedAgg[['name', 'value']],
        on='name', suffixes=('_baseline', '_optimized'), how='outer'
    )
    merged.rename(columns={
        'value_baseline': 'baseline',
        'value_optimized': 'optimized',
    }, inplace=True)
    merged['delta'] = merged['optimized'] - merged['baseline']
    merged['change_pct'] = (merged['delta'] / merged['baseline']) * 100

    if minDelta > 0:
        merged = merged[merged['delta'].abs() >= minDelta]

    return merged.sort_values('delta', ascending=True).reset_index(drop=True)


# =============================================================================
# TABLE OUTPUT
# =============================================================================

def printComparisonTable(data, title, baselineLabel='Baseline',
                         optimizedLabel='Optimized', unit='ms',
                         valueFormat='.2f', highlightNames=None):
    '''
    Print a formatted comparison table to stdout.

    Args:
        data: Comparison DataFrame (name, baseline, optimized, delta, change_pct)
        title: Section title printed above the table
        baselineLabel: Display name for the baseline column
        optimizedLabel: Display name for the optimized column
        unit: Unit suffix for values (e.g., "ms", " lines", " bytes")
        valueFormat: Format spec for values (e.g., ".2f", ".0f", ",d")
        highlightNames: Optional set of names to mark with *
    '''
    if data is None or data.empty:
        return

    if highlightNames is None:
        highlightNames = set()

    bCol = baselineLabel[:10]
    oCol = optimizedLabel[:10]

    print(f'\n{"=" * 85}')
    print(f'  {title}')
    print(f'{"=" * 85}')
    marker = ' *' if highlightNames else ''
    print(f"{'Name':<40} {bCol:>10} {oCol:>10} {'Delta':>10} {'Change':>8}{marker}")
    print('-' * 85)

    for _, row in data.iterrows():
        fullName = str(row['name'])
        name = fullName[:38]
        baseVal = row['baseline']
        optVal = row['optimized']
        deltaVal = row['delta']
        changePct = row['change_pct']

        affected = fullName in highlightNames
        mark = ' *' if affected else '  '

        baseStr = f'{baseVal:{valueFormat}}{unit}' if not isna(baseVal) else 'N/A'
        optStr = f'{optVal:{valueFormat}}{unit}' if not isna(optVal) else 'N/A'
        deltaStr = f'{deltaVal:+{valueFormat}}{unit}' if not isna(deltaVal) else 'N/A'
        changeStr = f'{changePct:+.1f}%' if not isna(changePct) else 'N/A'
        print(f'{name:<40} {baseStr:>10} {optStr:>10} {deltaStr:>10} {changeStr:>8}{mark}')

    print('-' * 85)

    improved = data[data['change_pct'] < 0]
    regressed = data[data['change_pct'] > 0]
    unchanged = data[data['change_pct'] == 0]
    validChanges = data.dropna(subset=['change_pct'])['change_pct']

    print(f'\nSummary: {len(improved)} improved, {len(regressed)} regressed, '
          f'{len(unchanged)} unchanged, {len(data)} total')

    if len(improved) > 0:
        best = improved.iloc[0]
        print(f"Best improvement: {best['name']} ({best['change_pct']:.1f}%)")

    if len(regressed) > 0:
        worst = regressed.iloc[-1]
        print(f"Worst regression: {worst['name']} ({worst['change_pct']:+.1f}%)")

    if len(validChanges) > 0:
        print(f'Overall: mean {validChanges.mean():+.1f}%, '
              f'median {validChanges.median():+.1f}%')

    if highlightNames:
        print(f'\n* = highlighted ({len(highlightNames)} items)')


# =============================================================================
# CHART OUTPUT
# =============================================================================

def createComparisonChart(data, outputPath, title,
                          baselineLabel='Baseline', optimizedLabel='Optimized',
                          unit='ms', highlightNames=None, highlightLabel=None,
                          subtitle=None):
    '''
    Create a paired before/after horizontal bar chart sorted by delta.

    Saves as SVG with searchable text.

    Args:
        data: Comparison DataFrame (name, baseline, optimized, delta, change_pct)
        outputPath: Path to save the chart (SVG)
        title: Chart title
        baselineLabel: Display name for the baseline series
        optimizedLabel: Display name for the optimized series
        unit: Unit suffix for value annotations
        highlightNames: Optional set of names to emphasise
        highlightLabel: Legend label for highlighted items
        subtitle: Optional subtitle line (e.g., filter parameters)
    '''
    if data is None:
        return

    if highlightNames is None:
        highlightNames = set()
    if not _have_matplotlib:
        logger.warning('Cannot create chart: matplotlib not installed.')
        return

    chartDf = data.dropna(subset=['baseline', 'optimized']).copy()
    if chartDf.empty:
        logger.warning('No data to chart')
        return

    # Reverse so largest improvements at TOP
    chartDf = chartDf.iloc[::-1].reset_index(drop=True)
    chartDf['is_highlighted'] = chartDf['name'].isin(highlightNames)

    def _makeLabel(row):
        name = row['name'][:28] + '...' if len(row['name']) > 28 else row['name']
        delta = row['delta']
        pct = row['change_pct']
        prefix = '* ' if row['is_highlighted'] else ''
        if pd.notna(delta) and pd.notna(pct):
            return f'{prefix}{name} ({delta:+.1f}{unit}, {pct:+.1f}%)'
        return f'{prefix}{name}'

    chartDf['display_name'] = chartDf.apply(_makeLabel, axis=1)

    figHeight = max(10, len(chartDf) * 0.5)
    fig, ax = plt.subplots(figsize=(14, figHeight))

    yPos = range(len(chartDf))
    barHeight = 0.35

    ax.barh([y + barHeight / 2 for y in yPos], chartDf['baseline'],
            barHeight, label=baselineLabel, color='#3498db', alpha=0.8)

    colors = ['#2ecc71' if d <= 0 else '#e74c3c' for d in chartDf['delta']]
    ax.barh([y - barHeight / 2 for y in yPos], chartDf['optimized'],
            barHeight, label=optimizedLabel, color=colors, alpha=0.8)

    for i, (b, o, delta) in enumerate(zip(chartDf['baseline'],
                                           chartDf['optimized'],
                                           chartDf['delta'])):
        ax.text(b + 1, i + barHeight / 2, f'{b:.1f}{unit}', va='center',
                fontsize=7, color='#2980b9')
        ax.text(o + 1, i - barHeight / 2, f'{o:.1f}{unit}', va='center',
                fontsize=7, color='#27ae60' if delta < 0 else '#c0392b')

    ax.set_yticks(yPos)
    ax.set_yticklabels(chartDf['display_name'])
    ax.set_xlabel(f'Value ({unit})' if unit else 'Value')

    if highlightNames:
        for i, (label, isHl) in enumerate(
                zip(ax.get_yticklabels(), chartDf['is_highlighted'])):
            if isHl:
                label.set_fontweight('bold')
                label.set_color('#8e44ad')

    titleLines = [title]
    if highlightLabel and highlightNames:
        titleLines.append(f'* = {highlightLabel}')
    if subtitle:
        titleLines.append(subtitle)
    ax.set_title('\n'.join(titleLines), fontsize=11)

    legendElements = [
        Patch(facecolor='#3498db', label=baselineLabel),
        Patch(facecolor='#2ecc71', label=f'{optimizedLabel} (improved)'),
        Patch(facecolor='#e74c3c', label=f'{optimizedLabel} (regressed)')
    ]
    ax.legend(handles=legendElements, loc='lower right')

    plt.tight_layout()
    plt.savefig(outputPath, format='svg', bbox_inches='tight')
    plt.close(fig)
    logger.info(f'Chart saved to: {outputPath}')


# =============================================================================
# HTML REPORT
# =============================================================================

def generateHtmlReport(reportPath, sections, pageTitle='Comparison Report',
                       subtitle=None, footerText='Generated by MaterialX diff tools'):
    '''
    Generate an HTML report with inline SVG charts (searchable text).

    Args:
        reportPath: Path to output HTML file
        sections: List of (title, chartPath) tuples. SVG chart files are read
                  and inlined so that text is searchable via Ctrl+F.
        pageTitle: Title for the HTML page header
        subtitle: Optional subtitle shown under the page title
        footerText: Footer attribution text
    '''
    reportPath = Path(reportPath)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)

    html = []
    html.append(f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{pageTitle}</title>
    <style>
        * {{ box-sizing: border-box; }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 0; padding: 20px; background: #f5f5f5;
        }}
        .container {{ max-width: 1800px; margin: 0 auto; }}
        h1, h2 {{ color: #333; }}
        h1 {{ border-bottom: 2px solid #3498db; padding-bottom: 10px; }}
        .subtitle {{ color: #666; font-size: 14px; margin-top: -8px; margin-bottom: 16px; }}
        .chart-section {{ background: white; border-radius: 8px; padding: 20px; margin-bottom: 30px;
                         box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .chart-section svg {{ max-width: 100%; height: auto; }}
    </style>
</head>
<body>
<div class="container">
    <h1>{pageTitle}</h1>
''')

    if subtitle:
        html.append(f'    <p class="subtitle">{subtitle}</p>\n')

    for title, chartFilePath in sections:
        chartFile = Path(chartFilePath) if chartFilePath else None
        if chartFile and chartFile.exists():
            svgContent = chartFile.read_text(encoding='utf-8')
            # Strip XML declaration if present (not needed when inlined)
            if svgContent.startswith('<?xml'):
                svgContent = svgContent[svgContent.index('?>') + 2:].lstrip()
            html.append(f'''
    <div class="chart-section">
        <h2>{title}</h2>
        {svgContent}
    </div>
''')

    html.append(f'''
    <footer style="margin-top: 40px; padding-top: 20px; border-top: 1px solid #ddd; color: #666; font-size: 12px;">
        {footerText}
    </footer>
</div>
</body>
</html>
''')

    with open(reportPath, 'w', encoding='utf-8') as f:
        f.write(''.join(html))

    logger.info(f'HTML report saved to: {reportPath}')


# =============================================================================
# PATH & BROWSER HELPERS
# =============================================================================

def chartPath(basePath, suffix):
    '''Derive a chart output path by inserting a suffix before the extension.'''
    basePath = Path(basePath)
    return basePath.parent / f'{basePath.stem}_{suffix}{basePath.suffix}'


def openReport(reportPath):
    '''Print the report path prominently and open it in the default browser.'''
    import webbrowser

    absPath = Path(reportPath).resolve()
    print(f'\n{"=" * 85}')
    print(f'  Report: {absPath}')
    print(f'{"=" * 85}')
    webbrowser.open(absPath.as_uri())
