#!/bin/bash

set -e

if [ "$1" = "-c" ]; then
    check=true
else
    check=false
fi

cd "$(dirname "$0")/../.."

nobindcall_files=()
bindcall_files=()

# shellcheck disable=SC2207
sorbet_runtime_files=(
    $(git ls-files -c -m -o --exclude-standard -- '*.rb' | \
             grep ^gems/sorbet-runtime/lib/
    )
)

for f in "${sorbet_runtime_files[@]}"; do
    if grep -E -q '^.*bind\([a-z]+).call' "$f"; then
        bindcall_files=("${bindcall_files[@]}" "$f")
        continue
    fi

    nobindcall_files=("${nobindcall_files[@]}" "$f")
done

mismatched=()
for f in "${bindcall_files[@]}"; do
    file_27="${f#.rb}_27.rb"
    origfile_27="${file_27}.orig"
    if [ -e "$file_27" ]; then
        mv "$file_27" "$origfile_27"
    fi
    perl -p -e 's/\.bind\(([a-z]+)\)\.call\(/.bind_call(\1, /g;' \
         -e 's/\.bind\(([a-z]+)\).call$/.bind_call(\1)/g;' \
         -e 's/^# typed: (true|strict)$/# typed: false/;' \
         -e 's/T\.absurd\(([a-z]+)\)/\1/' "$f" > "$file_27"

    if [ ! -e "$origfile_27" ]; then
        continue
    fi

    if ! cmp -s "$file_27" "$origfile_27"; then
        mismatched=("${mismatched[@]}" "$file_27")
    fi
    rm "${origfile_27}"
done

for f in "${nobindcall_files[@]}"; do
    file_27="${f#.rb}_27.rb"
    if [ ! -e "$file_27" ]; then
        continue
    fi

    rm "$file_27"
    mismatched=("${mismatched[@]}" "$file_27")
done

runtime_file=gems/sorbet-runtime/lib/sorbet-runtime.rb
for f in "${bindcall_files[@]}"; do
    relative=${f#gems/sorbet-runtime/lib/}
    relative=${relative%.rb}
    perl -p -i -e "s#^.*require_relative.*'${relative}'.*\$#require_relative USE_RUBY_27 ? '${relative}_27' : '${relative}'#" "$runtime_file"
done
for f in "${nobindcall_files[@]}"; do
    relative=${f#gems/sorbet-runtime/lib/}
    relative=${relative%.rb}
    perl -p -i -e "s#^.require_relative.*'${relative}'.*\$#require_relative '${relative}'#" "$runtime_file"
done

if [ "${#mismatched[@]}" -eq 0 ]; then
    exit 0
fi

if ! $check; then
    echo "Modified the following files:" >&2
else
    echo -ne "\\e[1;31m" >&2
    echo "Modified the following files!" >&2
    echo -ne "\\e[0m" >&2
    echo -e "Run \\e[97;1;42m ./tools/scripts/add_27_bindcall.sh\\e[0m to synchronize." >&2
fi

for f in "${mismatched[@]}"; do
    echo "$f" >&2
done

exit 1
