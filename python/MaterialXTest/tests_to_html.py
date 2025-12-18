#!/usr/bin/python

import sys
import os
import datetime
import argparse
import json

REDUCE_ENABLED = False
DIFF_ENABLED = False
try:
    # Load modules for image diff, resizing and base64 encoding.
    import base64
    import cv2
    import numpy as np
    REDUCE_ENABLED = True
    DIFF_ENABLED = True
except Exception:
    print("- OpenCV and NumPy need to be installed for image diff and base64 for reduced image features to be supported.")

class OpenCVImageUtils:
    def __init__(self):
        self.compute_reduced : bool = False
        self.reduced_width : int = 512
        self.difference_method = 'RMS'
        self.MAX_DIFFERENCE : float = 1.0
        self.hash_function = None
        self._reset()

    def _reset(self):
        self.difference_image : str = ""
        self.difference_value : float = self.MAX_DIFFERENCE
        self.difference_image : str = ""

    def set_reduced(self, compute_reduced: bool, width: int):
        self.compute_reduced = compute_reduced
        self.reduced_width = width
        if self.reduced_width <= 16:
            self.reduced_width = 16

    def set_difference_method(self, method: str):
        self.difference_method = method
        if method == 'COLORMOMENT':
            self.hash_function = cv2.img_hash.ColorMomentHash_create()
            print('- Setting hash function to: ' + method)
            self.MAX_DIFFERENCE = 100.0
        elif method == 'RMS':
            self.hash_function = None
            self.MAX_DIFFERENCE = 1.0
        else:
            self.hash_function = None
            self.MAX_DIFFERENCE = 1.0

    def get_difference_image(self) -> str:
        return self.difference_image
    
    def get_difference_value(self) -> float:
        return self.difference_value

    def compute_difference(self, image1Path, image2Path, imagediff_path_):        

        self._reset()

        if not image1Path or not image2Path:
            return False

        try:
            # Remove existing diff image if present
            if not self.compute_reduced and os.path.exists(imagediff_path_):
                os.remove(imagediff_path_)

            # Check input existence
            if not os.path.exists(image1Path):
                print("Image diff input missing: " + image1Path)
                return False
            if not os.path.exists(image2Path):
                print("Image diff input missing: " + image2Path)
                return False
            
            # Read images
            img1 = cv2.imread(image1Path, cv2.IMREAD_COLOR)
            img2 = cv2.imread(image2Path, cv2.IMREAD_COLOR)
            if img1 is None or img2 is None:
                print("Failed to read images.")
                return False

            # Resize to same dimensions
            if img1.shape != img2.shape:
                img2 = cv2.resize(img2, (img1.shape[1], img1.shape[0]))        

            # Compute absolute difference
            diff = cv2.absdiff(img1, img2)

            # Save diff image or compute reduced version
            if self.compute_reduced:
                self.difference_image = self.get_reduced_image_data_img(diff, self.reduced_width)  # Resize diff image for smaller size
            else:
                cv2.imwrite(imagediff_path_, diff)
                self.difference_image = imagediff_path_

            # Compute RMS per channel
            if self.difference_method == 'RMS':
                diff_float = diff.astype(np.float32)
                rms = np.sqrt(np.mean(np.square(diff_float), axis=(0, 1)))  # per channel
                self.difference_value = float(np.mean(rms) / 255.0)  # normalized average across RGB channels
                #print(f"RMS difference between images: {self.difference_value}")
                return True
            elif self.difference_method == 'COLORMOMENT' and self.hash_function is not None:
                hash1 = self.hash_function.compute(img1)
                hash2 = self.hash_function.compute(img2)
                self.difference_value = self.hash_function.compare(hash1, hash2)
                #print(f"Color Moments difference between images: {self.difference_value}")
                return True
            else:
                #print(f"Unsupported difference method: {self.difference_method}")
                return False

        except Exception as e:
            if not self.compute_difference and os.path.exists(imagediff_path_):
                os.remove(imagediff_path_)
            print(f"Failed to create image diff between: {image1Path}, {image2Path}")
            print(str(e))
        
        return False

    def get_reduced_image_data(self, image_path, width):
        if not image_path or not os.path.isfile(image_path):
            return None
        try:
            img = cv2.imread(image_path)
        except Exception:
            return None
        return self.get_reduced_image_data_img(img, width)

    def get_reduced_image_data_img(self, img, width):
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

def main(args=None):

    parser = argparse.ArgumentParser()
    parser.add_argument('-i1', '--inputdir1', dest='inputdir1', action='store', help='Input directory', default=".")
    parser.add_argument('-i2', '--inputdir2', dest='inputdir2', action='store', help='Second input directory', default="")
    parser.add_argument('-i3', '--inputdir3', dest='inputdir3', action='store', help='Third input directory', default="")
    parser.add_argument('-o', '--outputfile', dest='outputfile', action='store', help='Output file name', default="tests.html")
    parser.add_argument('-d', '--diff', dest='CREATE_DIFF', action='store_true', help='Perform image diff', default=False)
    parser.add_argument('-dm', '--diffmethod', dest='diffmethod', choices=['RMS','COLORMOMENT'], help='Image difference method to use. Currently only RMS is supported.', default='RMS')
    parser.add_argument('-e', '--error', dest='error', action='store', help='Filter out results with RMS less than this. Negative means all results are kept.', default=-1, type=float)
    parser.add_argument('-t', '--timestamp', dest='ENABLE_TIMESTAMPS', action='store_true', help='Write image timestamps', default=False)
    parser.add_argument('-w', '--imagewidth', type=int, dest='imagewidth', action='store', help='Set image display width. Default is 512. <= 0 means to resize dynamically', default=512)
    parser.add_argument('-l1', '--lang1', dest='lang1', action='store', help='First target language for comparison. Default is glsl', default="glsl")
    parser.add_argument('-l2', '--lang2', dest='lang2', action='store', help='Second target language for comparison. Default is osl', default="osl")
    parser.add_argument('-l3', '--lang3', dest='lang3', action='store', help='Third target language for comparison. Default is empty', default="")
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

    image_utils = None
    if args.CREATE_DIFF:
        if not DIFF_ENABLED: 
            print("--diff argument ignored. Diff utility not installed.")
        else:
            image_utils = OpenCVImageUtils()
    if args.reduced:
        if not REDUCE_ENABLED:
            print("--reduced argument ignored. Image reduction utility not installed.")
        else:
            if image_utils is None:
                image_utils = OpenCVImageUtils()

    difference_method = args.diffmethod
    if image_utils:
        print(f"Using difference method: '{difference_method}' and reduced images: '{args.reduced}' with width: '{args.imagewidth}'")
        image_utils.set_reduced(args.reduced, args.imagewidth)
        image_utils.set_difference_method(difference_method)

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
        for curFile in files:
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
    def prepend_file_uri(filepath: str, for_format: str) -> str:
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

                full_path_1 = os.path.join(path1, file1) if file1 else None
                full_path_2 = os.path.join(path2, file2) if file2 else None
                full_path_3 = os.path.join(path3, file3) if file3 else None
                diff_path_1 = diff_path_2 = diff_path_3 = None
                diff_value_1 = diff_value_2 = diff_value_3 = None

                if file1 and file2 and image_utils:
                    if full_path_1 and full_path_2:
                        base_prefix = full_path_1[:-8] if len(full_path_1) >= 8 else full_path_1
                        diff_path_1 = base_prefix + "_" + args.lang1 + "-1_vs_" + args.lang2 + "-2_diff.png"
                        if (image_utils.compute_difference(full_path_1, full_path_2, diff_path_1)):
                            diff_value_1 = image_utils.get_difference_value()
                            diff_path_1 = image_utils.get_difference_image()
                if useThirdLang and file1 and file3 and image_utils:
                    if full_path_1 and full_path_3:
                        base_prefix = full_path_1[:-8] if len(full_path_1) >= 8 else full_path_1
                        diff_path_2 = base_prefix + "_" + args.lang1 + "-1_vs_" + args.lang3 + "-3_diff.png"
                        if (image_utils.compute_difference(full_path_1, full_path_3, diff_path_2)):
                            diff_value_2 = image_utils.get_difference_value()
                            diff_path_2 = image_utils.get_difference_image()
                        diff_path_3 = base_prefix + "_" + args.lang2 + "-2_vs_" + args.lang3 + "-3_diff.png"
                        if (image_utils.compute_difference(full_path_2, full_path_3, diff_path_3)):
                            diff_value_3 = image_utils.get_difference_value()
                            diff_path_3 = image_utils.get_difference_image()

                # Row filtering based on tolerance:
                # - If error < 0: do not prune (always include rows)
                # - If error > 0: prune rows where all computed difference values are below the threshold
                if image_utils and args.error > 0:
                    diffs_present = []
                    if diff_value_1 is not None:
                        diffs_present.append(diff_value_1)
                    if diff_value_2 is not None:
                        diffs_present.append(diff_value_2)
                    if diff_value_3 is not None:
                        diffs_present.append(diff_value_3)
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
                    col = {"image": prepend_file_uri(image_path, args.format), "text": text}
                    if args.reduced and image_utils:
                        col["reduced_image"] = image_utils.get_reduced_image_data(image_path, args.imagewidth)
                    else:
                        col["reduced_image"] = None
                    return col

                if full_path_1:
                    text1 = file1
                    if args.ENABLE_TIMESTAMPS and os.path.isfile(full_path_1):
                        text1 += "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(full_path_1))) + ")"
                    columns.append(make_column(full_path_1, text1))

                if full_path_2:
                    text2 = file2
                    if args.ENABLE_TIMESTAMPS and os.path.isfile(full_path_2):
                        text2 += "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(full_path_2))) + ")"
                    columns.append(make_column(full_path_2, text2))

                if full_path_3:
                    text3 = file3
                    if args.ENABLE_TIMESTAMPS and os.path.isfile(full_path_3):
                        text3 += "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(full_path_3))) + ")"
                    columns.append(make_column(full_path_3, text3))

                def make_diff_column(diff_path, label, diff_value):
                    col = {"image": prepend_file_uri(diff_path, args.format), "text": label}
                    if args.reduced and image_utils:
                        col["reduced_image"] = image_utils.get_reduced_image_data(diff_path, args.imagewidth)
                    else:
                        col["reduced_image"] = None
                    return col

                if diff_path_1:
                    label = (f"{args.lang1.upper()} vs. {args.lang2.upper()} ({difference_method}: " + "%.5f)" % diff_value_1 ) if diff_value_1 is not None else ""
                    columns.append(make_diff_column(diff_path_1, label, diff_value_1))
                if diff_path_2:
                    label = (f"{args.lang1.upper()} vs. {args.lang3.upper()} ({difference_method}: " + "%.5f)" % diff_value_2 ) if diff_value_2 is not None else ""
                    columns.append(make_diff_column(diff_path_2, label, diff_value_2))
                if diff_path_3:
                    label = (f"{args.lang2.upper()} vs. {args.lang3.upper()} ({difference_method}: " + "%.5f)" % diff_value_3 ) if diff_value_3 is not None else ""
                    columns.append(make_diff_column(diff_path_3, label, diff_value_3))

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
                "diffMethod": args.diffmethod,
                "diffMax": args.diffmax,                
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
            html_parts.append("    <div class='h3 mb-4'>" + args.lang1 + " (in: " + args.inputdir1 + ") vs "+ args.lang2 + " (in: " + args.inputdir2 + ") vs "+ args.lang3 + " (in: " + args.inputdir3 + ")</div>\n")
        else:
            html_parts.append("    <div class='h3 mb-4'>" + args.lang1 + " (in: " + args.inputdir1 + ") vs "+ args.lang2 + " (in: " + args.inputdir2 + ")</div>\n")

        for group in groups:
            html_parts.append("    <div class='p-0 mb-0'>\n")
            html_parts.append("      <div class='text-break w-64' style='font-size:10pt; word-break:break-all;'>" + group["group"] + ":</div>\n")
            for row in group["rows"]:
                html_parts.append("      <div class='d-flex flex-nowrap align-items-start p-0 mb-0'>\n")
                for col in row["columns"]:
                    if args.imagewidth and args.imagewidth > 0:
                        html_parts.append("        <div class='border border-dark d-inline-block text-start me-0'>\n")
                    else:
                        html_parts.append("        <div class='border border-dark d-inline-block text-start me-0' style='width:100%;'>\n")
                    img_src = col.get("reduced_image") if args.reduced and col.get("reduced_image") else col.get("image")
                    if img_src:
                        html_parts.append("          <img src='" + img_src + "' class='test-image img-fluid' loading='lazy' alt='" + col.get("text", "").replace("<br>", " ") + "'/>")
                    html_parts.append("          <div class='text-break mt-0 mb-0 text-center' style='font-size:10pt'>" + col.get("text", "") + "</div>\n")
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
