# frozen_string_literal: true
# typed: true
# compiled: true

# Array#each_with_object is actually implemented with Array#each under the hood,
# and our intrinsics should respect that.

class Arraylike < Array
  def initialize
    @called = T.let(false, T::Boolean)
    super
  end

  def each_with_object(obj, &blk)
    @called = true
    super
  end
end

a = Arraylike["default"]

a.each_with_object(1) do |x, obj|
  p x
  p obj
end

p a.instance_variable_get(:@called)
