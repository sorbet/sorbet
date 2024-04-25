# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig
  sig {returns(Integer)}
  def foo
    91
  end
end

puts A.new.foo
