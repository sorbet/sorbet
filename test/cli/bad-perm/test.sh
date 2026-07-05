#!/bin/bash

dir=$(mktemp -d)
echo "parse_fail +" > "$dir/file.rb"
chmod 000 "$dir"
cmd="main/sorbet --silence-dev-message $dir"
if [ "$(id -u)" = 0 ]; then
    su -s /bin/bash nobody -c "$cmd" 2>&1 | sed "s,$dir,dir,"
else
    $cmd 2>&1 | sed "s,$dir,dir,"
fi
