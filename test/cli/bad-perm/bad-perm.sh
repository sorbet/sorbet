#!/bin/bash

dir=$(mktemp -d)
echo "parse_fail +" > "$dir/file.rb"
mkdir "$dir/no-read"
chmod 000 "$dir/no-read"
main/sorbet --silence-dev-message "$dir" 2>&1 | sed "s,$dir,dir,"
