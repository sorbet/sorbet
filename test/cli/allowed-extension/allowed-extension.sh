#!/bin/bash

dir="test/cli/allowed-extension/lib"

echo "Default extensions:"
main/sorbet --silence-dev-message $dir 2>&1 | grep -o "^[a-z.]\+" | sort

echo "------------------------------------------------------------------------"
echo "Override defaults:"
main/sorbet --silence-dev-message --allowed-extension .rb $dir 2>&1 | grep -o "^[a-z.]\+" | sort

echo "------------------------------------------------------------------------"
echo "Pass one custom extension:"
main/sorbet --silence-dev-message --allowed-extension .ru $dir 2>&1 | grep -o "^[a-z.]\+" | sort

echo "------------------------------------------------------------------------"
echo "Pass custom extensions:"
main/sorbet --silence-dev-message \
--allowed-extension .rb \
--allowed-extension .rbi \
--allowed-extension .ru \
--allowed-extension .rake \
$dir 2>&1 | grep -o "^[a-z.]\+" | sort
