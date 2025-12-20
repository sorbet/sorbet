# frozen_string_literal: true
# typed: true
# compiled: true

class Arraylike < Array
  def initialize
    @called = T.let(false, T::Boolean)
    super
  end

  def each(&blk)
    @called = true
    super
  end
end

a = Arraylike["default"]

a.each do |x|
  p x
end

p a.instance_variable_get(:@called)
