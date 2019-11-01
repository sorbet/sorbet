#!/bin/bash

dir=$(mktemp -d)
echo "T.reveal_type(1)" > "$dir/file.rb"
mkdir "$dir/no-read"
chmod 000 "$dir/no-read"
main/sorbet --silence-dev-message "$dir" 2>&1
