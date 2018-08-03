#!/bin/bash
main/sorbet -p error-files -q test/cli/files-with-errors/with-errors.rb test/cli/files-with-errors/without-errors.rb
