# frozen_string_literal: true
# typed: true
# compiled: true

# This is a test case designed to test some subtleties around argument handling
# and VM_METHOD_TYPE_IVAR methods.
#
# It fails (at time of writing) for reasons unrelated to the change to
# introduce a special case for VM_METHOD_TYPE_IVAR methods, so it must be
# disabled.

class A
  attr_reader :foo

  def wrap_foo(...)
    this = T.unsafe(self)
    this.foo(...)
  end

  def initialize
    @foo = 541
  end
end

puts A.new.wrap_foo
