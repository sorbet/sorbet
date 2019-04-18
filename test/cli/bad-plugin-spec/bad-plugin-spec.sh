#!/bin/bash
main/sorbet --dsl-plugins a_yaml_file_that_doesnt_exist -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/not-map.yaml -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/missing-triggers.yaml -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/triggers-not-map.yaml -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/ruby-extra-args-not-array.yaml -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/non-string-in-ruby-extra-args.yaml -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/values-not-scalar.yaml -e '' 2>&1
echo ---
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/duplicate-triggers.yaml -e '' 2>&1
