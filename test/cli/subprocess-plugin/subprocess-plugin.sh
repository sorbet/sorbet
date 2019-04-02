#!/bin/bash

# Check that the subprocess is getting good arguments
echo ------ Arguments
main/sorbet --silence-dev-message --dsl-plugins test/cli/subprocess-plugin/echo_argv.yaml --print plugin-generated-code test/cli/subprocess-plugin/permute.rb
echo ------ Multi file generated code
main/sorbet --silence-dev-message --dsl-plugins test/cli/subprocess-plugin/multi_empty.yaml --print plugin-generated-code test/cli/subprocess-plugin/multi*.rb
echo ------ Multi file symbol table
main/sorbet --silence-dev-message --dsl-plugins test/cli/subprocess-plugin/multi.yaml --print symbol-table test/cli/subprocess-plugin/multi*.rb
echo ------ Bad plugin output on single file
# we go through a different code path when there is a small number of files
main/sorbet --silence-dev-message --dsl-plugins test/cli/subprocess-plugin/bad_plugin.yaml test/cli/subprocess-plugin/trigger_bad_plugin.rb 2>&1
echo ------ Bad plugin output on many files
main/sorbet --silence-dev-message --dsl-plugins test/cli/subprocess-plugin/bad_plugin.yaml test/cli/subprocess-plugin/trigger_bad_plugin.rb a b c d e f 2>&1 \
  | grep 'Parse Error: unexpected token: syntax error'
