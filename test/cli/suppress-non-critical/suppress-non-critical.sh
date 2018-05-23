#!/bin/bash
main/sorbet -e '!' --suppress-non-critical 2>&1
echo $?
