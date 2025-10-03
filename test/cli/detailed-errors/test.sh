#!/usr/bin/env bash

cwd="$(pwd)"

cd test/cli/detailed-errors

"$cwd/main/sorbet" --silence-dev-message --max-threads=0 attached_class.rb hash.rb shapes.rb type_members.rb class_of.rb intersection.rb tuples.rb 2>&1
