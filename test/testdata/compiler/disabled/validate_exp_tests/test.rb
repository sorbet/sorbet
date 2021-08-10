# compiled: true
# frozen_string_literal: true
# typed: true

# This test is expected to fail, as its exp file has an error in it. This error
# will cause the llvm to fail to validate before a diff is performed. Failing in
# this manner wouldn't cause diff-diff.rb to report an error, so this exercises
# that case to ensure that it's failing properly.

puts "Hello!"
