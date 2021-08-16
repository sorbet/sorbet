#!/bin/bash
main/sorbet --silence-dev-message --cache-dir=nope2 -e '0' 2>&1
