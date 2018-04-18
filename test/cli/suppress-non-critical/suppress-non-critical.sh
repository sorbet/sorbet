#!/bin/bash
main/ruby-typer -e '!' --suppress-non-critical 2>&1
echo $?
