#!/bin/bash

unset SORBET_SILENCE_DEV_MESSAGE

main/sorbet -e 'x' 2>&1
echo '-----------------------------------------------------------------------'
main/sorbet --silence-dev-message -e 'x' 2>&1
echo '-----------------------------------------------------------------------'
export SORBET_SILENCE_DEV_MESSAGE=1
main/sorbet -e 'x' 2>&1
