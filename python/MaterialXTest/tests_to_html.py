#!/usr/bin/python

import sys
import os
import datetime
import argparse
import json

try:
    # Install pillow via pip to enable image differencing and statistics.
    from PIL import Image, ImageChops, ImageStat
    DIFF_ENABLED = True
except Exception:
    DIFF_ENABLED = False

try:
    # Install modules for image resizing and base64 encoding.
    import base64
    import cv2
    import numpy as np
    REDUCE_ENABLED = True
    DIFF_ENABLED = True
except Exception:
    REDUCE_ENABLED = False

def computeDiff(image1Path, image2Path, imageDiffPath, reduced=False, width=512):
    if not image1Path or not image2Path:
        return 1.0

    try:
        # Remove existing diff image if present
        if os.path.exists(imageDiffPath):
            os.remove(imageDiffPath)

        # Check input existence
        if not os.path.exists(image1Path):
            print("Image diff input missing: " + image1Path)
            return 1.0, ""
        if not os.path.exists(image2Path):
            print("Image diff input missing: " + image2Path)
            return 1.0, ""

        # Read images in color (BGR)
        img1 = cv2.imread(image1Path, cv2.IMREAD_COLOR)
        img2 = cv2.imread(image2Path, cv2.IMREAD_COLOR)
        if img1 is None or img2 is None:
            print("Failed to read images.")
            return 1.0, ""

        # Ensure both images have the same shape
        if img1.shape != img2.shape:
            print("Images have different dimensions or channels.")
            return 1.0, ""

        # Compute absolute difference
        diff = cv2.absdiff(img1, img2)

        # Save diff image (BGR, same as original OpenCV read)
        if reduced:
            imageDiffPath = get_reduced_image_data_img(diff, width)  # Resize diff image for smaller size
        else:
            cv2.imwrite(imageDiffPath, diff)

        # Compute RMS per channel (same as Pillow ImageStat.Stat(diff).rms)
        diff_float = diff.astype(np.float32)
        rms = np.sqrt(np.mean(np.square(diff_float), axis=(0, 1)))  # per channel
        return float(np.mean(rms) / 255.0), imageDiffPath  # normalized average across RGB channels

    except Exception as e:
        if not reduced and os.path.exists(imageDiffPath):
            os.remove(imageDiffPath)
        print(f"Failed to create image diff between: {image1Path}, {image2Path}")
        print(str(e))
    
    print('Returning default RMS of 1.0 due to error.')
    return 1.0, ""

def get_reduced_image_data(image_path, width):
    if not image_path or not os.path.isfile(image_path):
        return None
    try:
        img = cv2.imread(image_path)
    except Exception:
        return None
    return get_reduced_image_data_img(img, width)

def get_reduced_image_data_img(img, width):
    if img is None:
        return None
    try:
        h, w0 = img.shape[:2]
        w = width if width and width > 0 else 512
        aspect = h / w0
        new_size = (w, int(w * aspect))
        resized = cv2.resize(img, new_size, interpolation=cv2.INTER_AREA)
        # Encode as JPEG for smaller memory size
        ret, buf = cv2.imencode('.jpg', resized, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
        if not ret:
            return None
        b64 = base64.b64encode(buf.tobytes()).decode('utf-8')
        return f"data:image/jpeg;base64,{b64}"
    except Exception:
        return None

def computeDiff_PIL(image1Path, image2Path, imageDiffPath):
    if not image1Path or not image2Path:
        return 0
    try:
        if os.path.exists(imageDiffPath):
            os.remove(imageDiffPath)

        if not os.path.exists(image1Path):
            print ("Image diff input missing: " + image1Path)
            return

        if not os.path.exists(image2Path):
            print ("Image diff input missing: " + image2Path)
            return

        image1 = Image.open(image1Path).convert('RGB')
        image2 = Image.open(image2Path).convert('RGB')
        diff = ImageChops.difference(image1, image2)
        diff.save(imageDiffPath)
        diffStat = ImageStat.Stat(diff)
        return sum(diffStat.rms) / (3.0 * 255.0)
    except Exception:
        if os.path.exists(imageDiffPath):
            os.remove(imageDiffPath)
        print ("Failed to create image diff between: " + image1Path + ", " + image2Path)

def main(args=None):

    parser = argparse.ArgumentParser()
    parser.add_argument('-i1', '--inputdir1', dest='inputdir1', action='store', help='Input directory', default=".")
    parser.add_argument('-i2', '--inputdir2', dest='inputdir2', action='store', help='Second input directory', default="")
    parser.add_argument('-i3', '--inputdir3', dest='inputdir3', action='store', help='Third input directory', default="")
    parser.add_argument('-o', '--outputfile', dest='outputfile', action='store', help='Output file name', default="tests.html")
    parser.add_argument('-d', '--diff', dest='CREATE_DIFF', action='store_true', help='Perform image diff', default=False)
    parser.add_argument('-t', '--timestamp', dest='ENABLE_TIMESTAMPS', action='store_true', help='Write image timestamps', default=False)
    parser.add_argument('-w', '--imagewidth', type=int, dest='imagewidth', action='store', help='Set image display width. Default is 512. <= 0 means to resize dynamically', default=512)
    parser.add_argument('-l1', '--lang1', dest='lang1', action='store', help='First target language for comparison. Default is glsl', default="glsl")
    parser.add_argument('-l2', '--lang2', dest='lang2', action='store', help='Second target language for comparison. Default is osl', default="osl")
    parser.add_argument('-l3', '--lang3', dest='lang3', action='store', help='Third target language for comparison. Default is empty', default="")
    parser.add_argument('-e', '--error', dest='error', action='store', help='Filter out results with RMS less than this. Negative means all results are kept.', default=-1, type=float)
    parser.add_argument('-f', '--format', dest='format', choices=['html', 'json', 'markdown'], help='Output format: html, json, or markdown', default='html')
    parser.add_argument('-r', '--reduced', dest='reduced', action='store_true', help='Produce reduced-size images for display', default=False)

    args = parser.parse_args(args)
    # Build report data (groups -> rows -> columns), then render to desired format.

    if args.inputdir1 == ".":
        args.inputdir1 = os.getcwd()

    if args.inputdir2 == ".":
        args.inputdir2 = os.getcwd()
    elif args.inputdir2 == "":
        args.inputdir2 = args.inputdir1

    if args.inputdir3 == ".":
        args.inputdir3 = os.getcwd()
    elif args.inputdir3 == "":
        args.inputdir3 = args.inputdir1

    useThirdLang = args.lang3

    if not DIFF_ENABLED and args.CREATE_DIFF:
        print("--diff argument ignored. Diff utility not installed.")

    # Remove potential trailing path separators
    if args.inputdir1[-1:] == '/' or args.inputdir1[-1:] == '\\':
        args.inputdir1 = args.inputdir1[:-1]
    if args.inputdir2[-1:] == '/' or args.inputdir2[-1:] == '\\':
        args.inputdir2 = args.inputdir2[:-1]
    if args.inputdir3[-1:] == '/' or args.inputdir3[-1:] == '\\':
        args.inputdir3 = args.inputdir3[:-1]

    # Get all source files
    langFiles1 = []
    langPaths1 = []
    for subdir, _, files in os.walk(args.inputdir1):
        for curFile in sorted(files):
            if curFile.endswith(args.lang1 + ".png"):
                langFiles1.append(curFile)
                langPaths1.append(subdir)

    # Get all destination files, matching source files
    langFiles2 = []
    langPaths2 = []
    langFiles3 = []
    langPaths3 = []
    preFixLen: int = len(args.inputdir1) + 1  # including the path separator
    postFix: str = args.lang1 + ".png"
    for file1, path1 in zip(langFiles1, langPaths1):
        # Allow for just one language to be shown if source and dest are the same.
        # Otherwise add in equivalent name with dest language replacement if
        # pointing to the same directory
        if args.inputdir1 != args.inputdir2 or args.lang1 != args.lang2:
            file2 = file1[:-len(postFix)] + args.lang2 + ".png"
            path2 = os.path.join(args.inputdir2, path1[len(args.inputdir1)+1:])
        else:
            file2 = ""
            path2 = None
        langFiles2.append(file2)
        langPaths2.append(path2)

        if useThirdLang:
            file3 = file1[:-len(postFix)] + args.lang3 + ".png"
            path3 = os.path.join(args.inputdir2, path1[len(args.inputdir1)+1:])
        else:
            file3 = ""
            path3 = None
        langFiles3.append(file3)
        langPaths3.append(path3)

    # Helper to format image paths based on output format.
    # - html: use file:/// scheme for absolute paths
    # - markdown/json: use paths relative to the output file directory when possible
    def prependFileUri(filepath: str, for_format: str) -> str:
        if filepath is None:
            return None
        if os.path.isabs(filepath):
            if for_format == 'html':
                return 'file:///' + filepath
            else:
                out_dir = os.path.dirname(args.outputfile) if args.outputfile else os.getcwd()
                try:
                    rel = os.path.relpath(filepath, start=out_dir)
                    return rel
                except Exception:
                    return filepath
        else:
            return filepath

    # groups: list of { group: str, rows: [ { columns: [ { image: str, reduced_image: str, text: str } ] } ] }
    groups = []

    def build_groups():


        if langFiles1:
            curPath = ""
            current_group = None
            for file1, file2, file3, path1, path2, path3 in zip(langFiles1, langFiles2, langFiles3, langPaths1, langPaths2, langPaths3):

                fullPath1 = os.path.join(path1, file1) if file1 else None
                fullPath2 = os.path.join(path2, file2) if file2 else None
                fullPath3 = os.path.join(path3, file3) if file3 else None
                diffPath1 = diffPath2 = diffPath3 = None
                diffRms1 = diffRms2 = diffRms3 = None

                if file1 and file2 and DIFF_ENABLED and args.CREATE_DIFF:
                    if fullPath1 and fullPath2:
                        base_prefix = fullPath1[:-8] if len(fullPath1) >= 8 else fullPath1
                        diffPath1 = base_prefix + "_" + args.lang1 + "-1_vs_" + args.lang2 + "-2_diff.png"
                        diffRms1, diffPath1 = computeDiff(fullPath1, fullPath2, diffPath1, args.reduced, args.imagewidth)

                if useThirdLang and file1 and file3 and DIFF_ENABLED and args.CREATE_DIFF:
                    if fullPath1 and fullPath3:
                        base_prefix = fullPath1[:-8] if len(fullPath1) >= 8 else fullPath1
                        diffPath2 = base_prefix + "_" + args.lang1 + "-1_vs_" + args.lang3 + "-3_diff.png"
                        diffRms2, diffPath2 = computeDiff(fullPath1, fullPath3, diffPath2, args.reduced)
                        diffPath3 = base_prefix + "_" + args.lang2 + "-2_vs_" + args.lang3 + "-3_diff.png"
                        diffRms3, diffPath3 = computeDiff(fullPath2, fullPath3, diffPath3, args.reduced, args.imagewidth)

                # Row filtering based on tolerance:
                # - If error < 0: do not prune (always include rows)
                # - If error > 0: prune rows where all computed RMS values are below the threshold
                if args.CREATE_DIFF and DIFF_ENABLED and args.error > 0:
                    diffs_present = []
                    if diffRms1 is not None:
                        diffs_present.append(diffRms1)
                    if diffRms2 is not None:
                        diffs_present.append(diffRms2)
                    if diffRms3 is not None:
                        diffs_present.append(diffRms3)
                    # If we have at least one diff value and all are below the threshold, skip this row
                    if diffs_present and all(d < args.error for d in diffs_present):
                        continue

                # Detect group change and create group container (ensure not None)
                if current_group is None or curPath != path1:
                    current_group = {
                        "group": os.path.normpath(path1),
                        "rows": []
                    }
                    groups.append(current_group)
                    curPath = path1

                # Build columns for this row in the order: images (1..3) then diffs (1..3)
                columns = []

                def make_column(image_path, text):
                    col = {"image": prependFileUri(image_path, args.format), "text": text}
                    if args.reduced and REDUCE_ENABLED:
                        col["reduced_image"] = get_reduced_image_data(image_path, args.imagewidth)
                    else:
                        col["reduced_image"] = None
                    return col

                if fullPath1:
                    text1 = file1
                    if args.ENABLE_TIMESTAMPS and os.path.isfile(fullPath1):
                        text1 += "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullPath1))) + ")"
                    columns.append(make_column(fullPath1, text1))

                if fullPath2:
                    text2 = file2
                    if args.ENABLE_TIMESTAMPS and os.path.isfile(fullPath2):
                        text2 += "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullPath2))) + ")"
                    columns.append(make_column(fullPath2, text2))

                if fullPath3:
                    text3 = file3
                    if args.ENABLE_TIMESTAMPS and os.path.isfile(fullPath3):
                        text3 += "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullPath3))) + ")"
                    columns.append(make_column(fullPath3, text3))

                def make_diff_column(diff_path, label, rms):
                    col = {"image": prependFileUri(diff_path, args.format), "text": label}
                    if args.reduced and REDUCE_ENABLED:
                        col["reduced_image"] = get_reduced_image_data(diff_path, args.imagewidth)
                    else:
                        col["reduced_image"] = None
                    return col

                if diffPath1:
                    rms = (" (RMS " + "%.5f" % diffRms1 + ")") if diffRms1 is not None else ""
                    columns.append(make_diff_column(diffPath1, args.lang1.upper() + " vs. " + args.lang2.upper() + rms, diffRms1))
                if diffPath2:
                    rms = (" (RMS " + "%.5f" % diffRms2 + ")") if diffRms2 is not None else ""
                    columns.append(make_diff_column(diffPath2, args.lang1.upper() + " vs. " + args.lang3.upper() + rms, diffRms2))
                if diffPath3:
                    rms = (" (RMS " + "%.5f" % diffRms3 + ")") if diffRms3 is not None else ""
                    columns.append(make_diff_column(diffPath3, args.lang2.upper() + " vs. " + args.lang3.upper() + rms, diffRms3))

                current_group["rows"].append({"columns": columns})
            
    def output_json(groups):
        output = {
            "meta": {
                "inputdir1": args.inputdir1,
                "inputdir2": args.inputdir2,
                "inputdir3": args.inputdir3,
                "lang1": args.lang1,
                "lang2": args.lang2,
                "lang3": args.lang3,
                "createDiff": bool(args.CREATE_DIFF and DIFF_ENABLED),
                "timestamps": bool(args.ENABLE_TIMESTAMPS),
                "imagewidth": args.imagewidth,
                "tolerance": args.error,
            },
            "groups": groups
        }
        outputfile = args.outputfile.replace('.html', '.json') if args.outputfile.endswith('.html') else args.outputfile
        print('Writing JSON output to: ' + outputfile)
        with open(outputfile, "w+") as fh:
            json.dump(output, fh, indent=2)

    def output_markdown(groups):
        md_parts = []
        
        # Header
        if useThirdLang:
            md_parts.append("### " + args.lang1 + " (in: " + args.inputdir1 + ") vs " + args.lang2 + " (in: " + args.inputdir2 + ") vs " + args.lang3 + " (in: " + args.inputdir3 + ")\n\n")
        else:
            md_parts.append("### " + args.lang1 + " (in: " + args.inputdir1 + ") vs " + args.lang2 + " (in: " + args.inputdir2 + ")\n\n")
        
        # Render each group as a table
        for group in groups:
            md_parts.append("##### " + group["group"] + "\n\n")
            
            if not group["rows"]:
                continue
                
            # Determine number of columns from first row
            num_cols = len(group["rows"][0]["columns"]) if group["rows"] else 0
            
            if num_cols == 0:
                continue
            
            # Create markdown table header
            md_parts.append("|")
            for i in range(num_cols):
                md_parts.append("|")#" Column " + str(i+1) + " |")
            md_parts.append("\n")
            
            # Create separator row
            md_parts.append("|")
            for i in range(num_cols):
                md_parts.append(" --- |")
            md_parts.append("\n")
            
            # Render each row
            for row in group["rows"]:
                # Image row
                md_parts.append("|")
                for col in row["columns"]:
                    img_src = col.get("reduced_image") if args.reduced and col.get("reduced_image") else col.get("image")
                    if img_src:
                        md_parts.append(f" <img src='{img_src}' width='{args.imagewidth}' /> |")
                    else:
                        md_parts.append(" |")
                md_parts.append("\n")
                
                # Text row
                md_parts.append("|")
                for col in row["columns"]:
                    text = col.get("text", "")
                    # Replace '_" with ' ' to allow wrapping in Markdown viewers
                    text = text.replace("_", " ")
                    md_parts.append(" " + text + " |")
                md_parts.append("\n")
            
            md_parts.append("\n")
        
        outputfile = args.outputfile.replace('.html', '.md') if args.outputfile.endswith('.html') else args.outputfile
        print('Writing Markdown output to: ' + outputfile)
        with open(outputfile, "w+") as fh:
            fh.write(''.join(md_parts))        

    def output_html(groups):
        html_parts = []
        html_parts.append("<!DOCTYPE html>\n")
        html_parts.append("<html lang='en'>\n")
        html_parts.append("<head>\n")
        html_parts.append("  <meta charset='UTF-8'>\n")
        html_parts.append("  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n")
        html_parts.append("  <title>Test Results</title>\n")
        html_parts.append("  <link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css' rel='stylesheet' integrity='sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN' crossorigin='anonymous'>\n")
        html_parts.append("  <style>\n")
        html_parts.append("    .test-image {\n")
        html_parts.append("      background-color: black;\n")
        html_parts.append("      width: 100%;\n")
        html_parts.append("      height: auto;\n")
        if args.imagewidth and args.imagewidth > 0:
            html_parts.append("      max-width: " + str(args.imagewidth) + "px;\n")
        else:
            html_parts.append("      max-width: 100%;\n")
        html_parts.append("      object-fit: contain;\n")
        html_parts.append("    }\n")
        html_parts.append("  </style>\n")
        html_parts.append("</head>\n")
        html_parts.append("<body>\n")
        html_parts.append("  <div style='font-size:12pt;' class='small container-fluid py-4'>\n")

        if useThirdLang:
            html_parts.append("    <div class='h2 mb-4'>" + args.lang1 + " (in: " + args.inputdir1 + ") vs "+ args.lang2 + " (in: " + args.inputdir2 + ") vs "+ args.lang3 + " (in: " + args.inputdir3 + ")</div>\n")
        else:
            html_parts.append("    <div class='h2 mb-4'>" + args.lang1 + " (in: " + args.inputdir1 + ") vs "+ args.lang2 + " (in: " + args.inputdir2 + ")</div>\n")

        for group in groups:
            html_parts.append("    <div class='p-0 mb-0'>\n")
            html_parts.append("      <div class='text-break w-64' style='font-size:10pt; word-break:break-all;'>" + group["group"] + ":</div>\n")
            for row in group["rows"]:
                html_parts.append("      <div class='d-flex flex-nowrap align-items-start px-2 mb-0'>\n")
                for col in row["columns"]:
                    if args.imagewidth and args.imagewidth > 0:
                        html_parts.append("        <div class='border border-dark d-inline-block text-start me-0'>\n")
                    else:
                        html_parts.append("        <div class='border border-dark d-inline-block text-start me-0' style='width:100%;'>\n")
                    img_src = col.get("reduced_image") if args.reduced and col.get("reduced_image") else col.get("image")
                    if img_src:
                        html_parts.append("          <img src='" + img_src + "' class='test-image img-fluid' loading='lazy' alt='" + col.get("text", "").replace("<br>", " ") + "'/>")
                    html_parts.append("          <div class='text-break mt-0 mb-0' style='font-size:10pt'>" + col.get("text", "") + "</div>\n")
                    html_parts.append("        </div>\n")
                html_parts.append("      </div>\n")
            html_parts.append("    </div>\n")

        html_parts.append("  </div>\n")
        html_parts.append("  <script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js' integrity='sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL' crossorigin='anonymous'></script>\n")
        html_parts.append("</body>\n")
        html_parts.append("</html>\n")

        print('Writing HTML output to: ' + args.outputfile)
        with open(args.outputfile, "w+") as fh:
            fh.write(''.join(html_parts))        

    # Build groups data structure
    build_groups()

    # Render output: JSON, Markdown, or HTML
    if args.format == 'json':
        output_json(groups)

    elif args.format == 'markdown':
        output_markdown(groups)
    else:
        output_html(groups)

if __name__ == "__main__":
    main(sys.argv[1:])
