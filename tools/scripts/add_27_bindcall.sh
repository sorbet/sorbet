#!/bin/bash

set -e

if [ "$1" = "-c" ]; then
    check=true
else
    check=false
fi

cd "$(dirname "$0")/../.."

bindcall_files=()

for f in $(find gems/sorbet-runtime/lib -name \*.rb); do
    if ! egrep -E -q '^.*bind\([a-z]+).call' $f; then
        continue
    fi
    bindcall_files=("${bindcall_files[@]}" "$f")
done

mismatched=()
for f in "${bindcall_files[@]}"; do
    file_27="${f#.rb}_27.rb"
    origfile_27="${file_27}.orig}"
    if [ -e "$file_27" ]; then
        mv "$file_27" "$origfile_27"
    fi
    perl -p -e 's/\.bind\(([a-z]+)\)\.call\(/.bind_call(\1, /g;' \
     -e 's/\.bind\(([a-z]+)\).call$/.bind_call(\1)/g' "$f" > "$file_27"

    if ! $check; then
        continue
    fi

    if [ -e "$origfile_27" ]; then
        if ! cmp -s "$file_27" "$origfile_27"; then
            mismatched=("${mismatched[@]}" "$file_27")
        fi
        rm "${origfile_27}"
    fi
done

runtime_file=gems/sorbet-runtime/lib/sorbet-runtime.rb
for f in "${bindcall_files[@]}"; do
    relative=${f#gems/sorbet-runtime/lib/}
    relative=${relative%.rb}
    perl -p -i -e "s#^.*require_relative.*'${relative}'.*\$#require_relative USE_RUBY_27 ? '${relative}_27' : '${relative}'#" ${runtime_file}
done

if [ "${#mismatched[@]}" -eq 0 ]; then
    exit 0
fi

if ! $check; then
    echo "Added the following files:" >&2
else
    echo -ne "\\e[1;31m" >&2
    echo "Added bind_call usages to the following files!" >&2
    echo -ne "\\e[0m" >&2
    echo -e "Run \\e[97;1;42m ./tools/scripts/add_27_bindcall.sh \\e[0m to format." >&2
fi

for f in "${mismatched[@]}"; do
    echo "$f" >&2
done

exit 1
