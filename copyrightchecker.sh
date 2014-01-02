#!/bin/sh

python3 -B license.py c > expected_copyright

source_files=`find . '(' -name '*.cc' -o -name '*.hh' -o -name '*.vert' -o -name '*.geom' -o -name '*.frag' ')'`

for file in $source_files
do
    echo "Checking $file"
    awk 'BEGIN { p=1 }; { if ($0 == "") p=0; if (p) print $0 }' < $file > extracted_copyright
    diff expected_copyright extracted_copyright
done

python3 -B license.py shell > expected_copyright

source_files="Makefile `find . '(' -name '*.sh' -o -name '*.py' ')'`"

for file in $source_files
do
    echo "Checking $file"
    awk 'BEGIN { p=1 }; { if ($0 == "") p=0; if (p) print $0 }' < $file > extracted_copyright
    diff expected_copyright extracted_copyright
done

rm -f expected_copyright extracted_copyright
