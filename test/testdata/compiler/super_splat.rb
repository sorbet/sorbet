# compiled: true
# typed: true
# frozen_string_literal: true

class Parent
  extend T::Sig

  def foo(*args)
    puts args
  end

end

class Child < Parent
  extend T::Sig

  def foo(*args)
    super(*T.unsafe(args))
  end

end

args = [1,2,3]
Child.new.foo(*[1,2,3])
