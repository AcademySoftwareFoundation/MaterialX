#!/usr/bin/python

import sys
import os
import datetime
import argparse
import itertools

try:
    # Use pip to install Pillow and Image to enable image diffs
    from PIL import Image
    from PIL import ImageChops
    DIFF_ENABLED = True
except Exception:
    DIFF_ENABLED = False

try:
    from itertools import zip_longest
except ImportError:
    from itertools import izip_longest as zip_longest

def createDiff(image1Path, image2Path, imageDiffPath):
    try:
        image1 = Image.open(image1Path).convert('RGB')
        image2 = Image.open(image2Path).convert('RGB')
        diff = ImageChops.difference(image1, image2)
        diff.save(imageDiffPath)
    except Exception:
        print ("Failed to create image diff between: " + image1Path + ", " + image2Path)

def main(args=None):

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputdir', dest='inputdir', action='store', help='Input directory', default=".")
    parser.add_argument('-o', '--outputfile', dest='outputfile', action='store', help='Output file name', default="tests.html")
    parser.add_argument('-d', '--diff', dest='CREATE_DIFF', action='store_true', help='Perform image diff', default=False)
    parser.add_argument('-t', '--timestamp', dest='ENABLE_TIMESTAMPS', action='store_true', help='Write image timestamps', default=False)
    parser.add_argument('-w', '--imagewidth', type=int, dest='imagewidth', action='store', help='Set image display width', default=256)
    parser.add_argument('-cp', '--cellpadding', type=int, dest='cellpadding', action='store', help='Set table cell padding', default=0)
    parser.add_argument('-tb', '--tableborder', type=int, dest='tableborder', action='store', help='Table border width. 0 means no border', default=3)
    args = parser.parse_args(args)

    fh = open(args.outputfile,"w+")
    fh.write("<html>\n")
    fh.write("<style>\n")
    fh.write("td {")
    fh.write("    padding: " + str(args.cellpadding) + ";")
    fh.write("    border: " + str(args.tableborder) + "px solid black;")
    fh.write("}")
    fh.write("table, tbody, th, .td_image {")
    fh.write("    border-collapse: collapse;")
    fh.write("    padding: 0;")
    fh.write("    margin: 0;")
    fh.write("}")
    fh.write("</style>")
    fh.write("<body>\n")

    # Iterate over subdirectories
    for subdir, _, files in os.walk(args.inputdir):
        glslFiles = []
        oslFiles = []
        for curFile in files:
            if curFile.endswith("glsl.png"):
                glslFiles.append(curFile)
            if curFile.endswith("osl.png"):
                oslFiles.append(curFile)
        if len(glslFiles) != len(oslFiles):
            print ("Number of glsl files " + str(len(glslFiles)) + " does not match number of osl files " +  str(len(oslFiles)) + " in dir: " + subdir)
            print ("GLSL list: " + str(glslFiles))
            print ("OSL files: " + str(oslFiles))
        if len(glslFiles) > 0 and len(oslFiles) > 0:
            fh.write("<h3>" + subdir + ":</h3>\n")
            fh.write("<table>\n")
            for glslFile, oslFile in zip_longest(glslFiles, oslFiles):
                fullGlslPath = os.path.join(subdir, glslFile) if glslFile else None
                fullOslPath = os.path.join(subdir, oslFile) if glslFile else None
                if glslFile and oslFile and DIFF_ENABLED and args.CREATE_DIFF:
                    diffPath = fullGlslPath[0:-8] + "diff.png"
                    createDiff(fullGlslPath, fullOslPath, diffPath)
                else:
                    diffPath = None
                fh.write("    <tr>\n")
                if fullGlslPath:
                    fh.write("        <td class='td_image'><img src='" + fullGlslPath + "' height='" + str(args.imagewidth) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
                if fullOslPath:
                    fh.write("        <td class='td_image'><img src='" + fullOslPath + "' height='" + str(args.imagewidth) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
                if diffPath:
                    fh.write("        <td class='td_image'><img src='" + diffPath + "' height='" + str(args.imagewidth) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
                fh.write("    </tr>\n")
                fh.write("    <tr>\n")
                if fullGlslPath:
                    if args.ENABLE_TIMESTAMPS:
                        fh.write("        <td align='center'>" + glslFile + "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullGlslPath))) + ")</td>\n")
                    else:
                        fh.write("        <td align='center'>" + glslFile + "</td>\n")
                if fullOslPath:
                    if args.ENABLE_TIMESTAMPS:
                        fh.write("        <td align='center'>" + oslFile + "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullOslPath))) + ")</td>\n")
                    else:
                        fh.write("        <td align='center'>" + oslFile + "</td>\n")
                if diffPath:
                    fh.write("        <td align='center'>" + glslFile[0:-8] + "diff.png" + "</td>\n")
                fh.write("    </tr>\n")
            fh.write("</table>\n")

    fh.write("</body>\n")
    fh.write("</html>\n")

if __name__ == "__main__":
    main(sys.argv[1:])
