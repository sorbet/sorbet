#!/usr/bin/env bash

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

dir_grep() {
    grep -r "$1" "$2" | grep -o "\(core\|stdlib\).*"
}

# Can't use `--print=payload-sources` with `--no-stdlib`
main/sorbet --silence-dev-message --print=payload-sources:$dir --no-stdlib 2>&1

echo --------------------------------------------------------------------------

# Can't use `--print=payload-sources` with `-e`
main/sorbet --silence-dev-message --print=payload-sources:$dir -e "foo" 2>&1

echo --------------------------------------------------------------------------

# Can't use `--print=payload-sources` with arguments
main/sorbet --silence-dev-message --print=payload-sources:$dir file.rb 2>&1

echo --------------------------------------------------------------------------

# Dump payload even if the directory exists
main/sorbet --silence-dev-message --print=payload-sources:$dir
dir_grep "class Array < Object" $dir
dir_grep "class Object < BasicObject" $dir
dir_grep "module Base64" $dir

echo --------------------------------------------------------------------------

# Dump payload even if the directory doesn't exist
mkdir -p $dir/subdir1/subdir2
main/sorbet --silence-dev-message --print=payload-sources:$dir/subdir1/subdir2
dir_grep "class Array < Object" $dir/subdir1/subdir2
dir_grep "class Object < BasicObject" $dir/subdir1/subdir2
dir_grep "module Base64" $dir/subdir1/subdir2
