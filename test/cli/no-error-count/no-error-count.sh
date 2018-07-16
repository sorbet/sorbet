#!/bin/bash
main/sorbet -e '1' --no-error-count 2>&1
main/sorbet -e '!' --no-error-count 2>&1
