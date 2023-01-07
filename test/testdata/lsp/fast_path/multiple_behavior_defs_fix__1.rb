# typed: false
# stripe-mode: true
# spacer for assert-fast-path
# spacer for exclude-from-file-update

# This error does not go away on the fast path, even though it should.

module Foo # error: has behavior defined in multiple files
  def method1; end
end
