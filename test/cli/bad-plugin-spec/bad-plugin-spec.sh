#!/bin/bash
main/sorbet --dsl-plugins a_yaml_file_that_doesnt_exist -e '' 2>&1
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/not-map.yaml -e '' 2>&1
main/sorbet --dsl-plugins test/cli/bad-plugin-spec/values-not-scalar.yaml -e '' 2>&1
