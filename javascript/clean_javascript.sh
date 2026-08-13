#!/usr/bin/env bash
# This script cleans the JavaScript build area on macOS / Linux.
# Run from the Github root folder.
rm -rf javascript/build
rm -rf javascript/node_modules
rm -rf javascript/MaterialXTest/_build
rm -rf javascript/MaterialXTest/playwright-report
rm -rf javascript/MaterialXView/node_modules
rm -rf javascript/MaterialXView/dist
