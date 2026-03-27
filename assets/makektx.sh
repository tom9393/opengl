#!/bin/bash

for file in *.jpg; do
    # Skip if no png files are found
    [ -e "$file" ] || continue
    
    # Strip the .png extension to get the base name
    filename="${file%.*}"
    
    echo "Processing: $file"
    toktx --t2 --genmipmap --bcmp "${filename}.ktx2" "$file"
done
