# typed: true

require 'sorbet-runtime'

class A
  def custom?
    true
  end
end

class NilClass
  def custom?
    false
  end
end

a = T.let(A.new, T.nilable(A))
if a.custom?
  T.reveal_type(a)
end

class Super
  extend T::Sig
  extend T::Helpers

  abstract!

  # SubAがサブクラスであること
  # trueならclass変える
  sig { abstract.returns(T::Boolean).narrows_to(SubA) }
  def a?; end
end

class SubA < Super
  sig { override.returns(TrueClass).narrows_to(SubA) }
  def a?
    pp(true)
  end
end

class SubB < Super
  sig { override.returns(FalseClass).narrows_to(SubA) }
  def a?
    pp(false)
  end
end

foo = T.let(SubA.new, Super)
if foo.a?
  T.reveal_type(foo)
end

unless foo.nil?
  T.reveal_type(foo)
end
