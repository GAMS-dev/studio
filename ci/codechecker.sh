#!/usr/bin/env bash

qmake gams-studio.pro CONFIG+=release

# Log your project.
CodeChecker log -b "make -j4" -o compilation_database.json

CodeChecker analyze           \
  --analyzers clang-tidy      \
  --jobs 4                    \
  -i ci/skipfile.txt          \
  -o reports                  \
  compilation_database.json

# Create the report file by using the CodeChecker parse command.
CodeChecker parse             \
  --trim-path-prefix $(pwd)   \
  -e codeclimate              \
  reports > gl-code-quality-report.json

exit 0
