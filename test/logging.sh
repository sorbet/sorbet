#!/usr/bin/env bash

red=$'\x1b[0;31m'
green=$'\x1b[0;32m'
yellow=$'\x1b[0;33m'
cyan=$'\x1b[0;36m'
cnone=$'\x1b[0m'

# Unconditionally adds colors (regardless of whether output is a tty)
in_color() {
  local color="$1"
  shift

  echo "$color$*$cnone"
}

success() { echo "$(in_color "$green" "[ OK ]") $*"; }
error()   { echo "$(in_color "$red"   "[ERR!]") $*"; }
info()    { echo "$(in_color "$cyan"  "[ .. ]") $*"; }
# Color entire line to get users' attention (because we won't stop).
attn()    { in_color "$yellow" "[ .. ] $*"; }

fatal() {
  error "$@"
  exit 1
}
