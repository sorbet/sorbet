#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message -e '1.to_' 2>&1
