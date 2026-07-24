@echo off
python tests_to_html.py -i1 ../../build --order-from "%~dp0..\..\resources\Materials\TestSuite\_options.mtlx" %* -d
