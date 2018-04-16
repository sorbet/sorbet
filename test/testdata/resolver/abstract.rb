# @typed

class HasAbstract
  abstract!

  sig(x: Integer).abstract.returns(String)
  def abstract(x)
  end

  sig.abstract
  def self.abstract_singleton
  end

  sig(x: Integer).abstract.returns(String)
  def abstract_with_body(x)
    14 # error: Abstract methods must not contain any code in their body.
  end

end

class InterfaceClass
  interface! # error: Classes can't be interfaces
end

class AbstractMethodNotClass
  sig.returns(T.untyped).abstract
  def f; end # error: you must mark your class/module as abstract
end

module InterfaceModule
  interface!

  def f # error: must be declared abstract
    0
  end
end
