#!/bin/bash


for file in `find . -type f -name "*.png"`; do
  dest="${file%.*}.webp"
  echo "$file --> $dest"
  magick $file -resize 1280x $dest
done
