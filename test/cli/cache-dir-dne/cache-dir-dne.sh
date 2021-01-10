#!/bin/bash
main/sorbet --silence-dev-message --cache-dir=nope -e '0' 2>&1
