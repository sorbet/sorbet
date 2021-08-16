# compiled: true
# frozen_string_literal: true
# typed: true

# This test is expected to fail, as its exp file has an error in it. This error
# will cause the llvm to fail to validate before a diff is performed. Failing in
# this manner wouldn't cause diff-diff.rb to report an error, so this exercises
# that case to ensure that it's failing properly.

# The oracle test needs to fail as well, but it's not the failure we're
# interested in. Since the tests will never be run at the same time and their
# output will be compared, it's sufficient to print out the current time to
# guarantee a failure for the oracle test.
puts Time.new
