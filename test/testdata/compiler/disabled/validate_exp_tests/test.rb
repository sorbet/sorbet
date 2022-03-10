# compiled: true
# frozen_string_literal: true
# typed: true

# This test is expected to fail, as its exp file has an error in it. This error
# will cause the llvm to fail to validate before a diff is performed. Failing in
# this manner wouldn't cause diff-diff.rb to report an error, so this exercises
# that case to ensure that it's failing properly.

# The oracle test needs to fail as well, but it's not the failure we're
# interested in. We used to compare the output of `Time.new`, but empirical
# evidence says that sometimes the compiled version and the interpreted version
# can print out the same time. Instead, we'll use this fancy compiler intrinsic
# which is guaranteed to produce different output in each scenario.
puts T::Private::Compiler.running_compiled?
