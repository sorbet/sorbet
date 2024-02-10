#!/usr/bin/env bash
bundle exec ruby entry.rb
echo $'\n\n^unpatched ---------- patched:\n\n'
PATCH_LOAD_ISEQ=1 bundle exec ruby entry.rb
