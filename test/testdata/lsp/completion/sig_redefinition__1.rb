# typed: true

class Module
  include T::Sig
end

class A
  sig # error: no block
  #  ^ apply-completion: [A] item: 0
  def foo; end

  def bar; end
end
