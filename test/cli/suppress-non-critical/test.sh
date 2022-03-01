#!/bin/bash
main/sorbet --silence-dev-message -e '!' --suppress-non-critical 2>&1
echo $?
