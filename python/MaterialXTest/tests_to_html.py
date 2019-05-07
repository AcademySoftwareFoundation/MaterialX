#!/usr/bin/python

import sys
import getopt
import os
import datetime

try:
    # Use pip to install Pillow and Image to enable image diffs
    from PIL import Image
    from PIL import ImageChops
    DIFF_ENABLED = True
except Exception:
    DIFF_ENABLED = False

def createDiff(image1Path, image2Path, imageDiffPath):
    try:
        image1 = Image.open(image1Path).convert('RGB')
        image2 = Image.open(image2Path).convert('RGB')
        diff = ImageChops.difference(image1, image2)
        diff.save(imageDiffPath)
    except Exception:
        print "Failed to create image diff between: " + image1Path + ", " + image2Path

def main(argv):
    inputdir = '.'
    outputfile = "tests.html"
    CREATE_DIFF = False
    ENABLE_TIMESTAMPS = False
    try:
        opts, _ = getopt.getopt(argv,"hi:o:dt",["idir=", "ofile="])
    except getopt.GetoptError:
        print 'tests_to_html.py -i <inputdir> -o <outputfile> -d -t'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'tests_to_html.py -i <inputdir> -o <outputfile> -d -t'
            sys.exit()
        elif opt in ("-i", "--idir"):
            inputdir = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
        elif opt in ("-d"):
            CREATE_DIFF = True
        elif opt in ("-t"):
            ENABLE_TIMESTAMPS = True

    fh = open(outputfile,"w+")
    fh.write("<html>\n")
    fh.write("<style>\n")
    fh.write("td {")
    fh.write("    padding: 10;")
    fh.write("    border: 3px solid black;")
    fh.write("}")
    fh.write("table, tbody, th, .td_image {")
    fh.write("    border-collapse: collapse;")
    fh.write("    padding: 0;")
    fh.write("    margin: 0;")
    fh.write("}")
    fh.write("</style>")
    fh.write("<body>\n")

    # Iterate over subdirectories
    for subdir, _, files in os.walk(inputdir):
        glslFiles = []
        oslFiles = []
        for curFile in files:
            if curFile.endswith("glsl.png"):
                glslFiles.append(curFile)
            if curFile.endswith("osl.png"):
                oslFiles.append(curFile)
        if len(glslFiles) != len(oslFiles):
            print ("Number of glsl files does not match number of osl files in dir: " + subdir)
            continue
        elif len(glslFiles) > 0 and len(oslFiles) > 0:
            fh.write("<h1>" + subdir + ":</h1><br>\n")
            fh.write("<table>\n")
            for glslFile, oslFile in zip(glslFiles, oslFiles):
                fullGlslPath = os.path.join(subdir, glslFile)
                fullOslPath = os.path.join(subdir, oslFile)
                if glslFile and oslFile and DIFF_ENABLED and CREATE_DIFF:
                    diffPath = fullGlslPath[0:-8] + "diff.png"
                    createDiff(fullGlslPath, fullOslPath, diffPath)
                else:
                    diffPath = None
                fh.write("    <tr>\n")
                if glslFile:
                    fh.write("        <td class='td_image'><img src='" + fullGlslPath + "' style='background-color:black;'/></td>\n")
                if oslFile:
                    fh.write("        <td class='td_image'><img src='" + fullOslPath + "' style='background-color:black;'/></td>\n")
                if diffPath:
                    fh.write("        <td class='td_image'><img src='" + diffPath + "' style='background-color:black;'/></td>\n")
                fh.write("    </tr>\n")
                fh.write("    <tr>\n")
                if glslFile:
                    if ENABLE_TIMESTAMPS:
                        fh.write("        <td align='center'>" + glslFile + "(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullGlslPath))) + ")</td>\n")
                    else:
                        fh.write("        <td align='center'>" + glslFile + "</td>\n")
                if oslFile:
                    if ENABLE_TIMESTAMPS:
                        fh.write("        <td align='center'>" + oslFile + "(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullOslPath))) + ")</td>\n")
                    else:
                        fh.write("        <td align='center'>" + oslFile + "</td>\n")
                if diffPath:
                    fh.write("        <td align='center'>" + glslFile[0:-8] + "diff.png" + "</td>\n")
                fh.write("    </tr>\n")
            fh.write("</table><br><br>\n")

    fh.write("</body>\n")
    fh.write("</html>\n")

if __name__ == "__main__":
    main(sys.argv[1:])
