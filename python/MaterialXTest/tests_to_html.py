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
    parser.add_argument('-sl', '--sourcelang', dest='sourcelang', action='store', help='Source language. Default is source', default="glsl")
    parser.add_argument('-dl', '--destlang', dest='destlang', action='store', help='Destination language. Default is dest', default="osl")

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
        sourceFiles = []
        destFiles = []
        for curFile in files:
            if curFile.endswith(args.sourcelang + ".png"):
                sourceFiles.append(curFile)
            if curFile.endswith(args.destlang + ".png"):
                destFiles.append(curFile)
        if len(sourceFiles) != len(destFiles):
            #print ("Number of source files " + str(len(sourceFiles)) + " does not match number of dest files " +  str(len(destFiles)) + " in dir: " + subdir)
            print ("source list: " + str(sourceFiles))
            print ("dest files: " + str(destFiles))
        if len(sourceFiles) > 0 and len(destFiles) > 0:
            fh.write("<h3>" + subdir + ":</h3>\n")
            fh.write("<table>\n")
            for sourceFile, destFile in zip_longest(sourceFiles, destFiles):
                fullsourcePath = os.path.join(subdir, sourceFile) if sourceFile else None
                fulldestPath = os.path.join(subdir, destFile) if destFile else None
                if sourceFile and destFile and DIFF_ENABLED and args.CREATE_DIFF:
                    diffPath = fullsourcePath[0:-8] + "diff.png"
                    createDiff(fullsourcePath, fulldestPath, diffPath)
                else:
                    diffPath = None
                fh.write("    <tr>\n")
                if fullsourcePath:
                    fh.write("        <td class='td_image'><img src='" + fullsourcePath + "' height='" + str(args.imagewidth) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
                if fulldestPath:
                    fh.write("        <td class='td_image'><img src='" + fulldestPath + "' height='" + str(args.imagewidth) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
                if diffPath:
                    fh.write("        <td class='td_image'><img src='" + diffPath + "' height='" + str(args.imagewidth) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
                fh.write("    </tr>\n")
                fh.write("    <tr>\n")
                if fullsourcePath:
                    if args.ENABLE_TIMESTAMPS:
                        fh.write("        <td align='center'>" + sourceFile + "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullsourcePath))) + ")</td>\n")
                    else:
                        fh.write("        <td align='center'>" + sourceFile + "</td>\n")
                if fulldestPath:
                    if args.ENABLE_TIMESTAMPS:
                        fh.write("        <td align='center'>" + destFile + "<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fulldestPath))) + ")</td>\n")
                    else:
                        fh.write("        <td align='center'>" + destFile + "</td>\n")
                if diffPath:
                    fh.write("        <td align='center'>" + sourceFile[0:-8] + "diff.png" + "</td>\n")
                fh.write("    </tr>\n")
            fh.write("</table>\n")

    fh.write("</body>\n")
    fh.write("</html>\n")

if __name__ == "__main__":
    main(sys.argv[1:])
