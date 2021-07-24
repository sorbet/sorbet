# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig

  sig {params(x: T::Hash[Symbol, String], y: T.nilable(String)).returns(Integer)}
  def foo(x, y: nil)
    639
  end
end

p A.new.foo({})
