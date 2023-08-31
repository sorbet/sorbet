# typed: true
module Interface
  extend T::Sig
  extend T::Helpers

  interface!

  sig {abstract.returns(T.untyped)}
  def a_method; end
end

module InterfaceBadChild # error: Missing definition for abstract method `Interface#a_method`
  include Interface
end

module InterfaceChild
  extend T::Sig
  extend T::Helpers

  interface!
  include Interface
end

module ConcreteMixin
  include Interface

  def a_method; end
end

class Abstract
  extend T::Sig
  extend T::Helpers

  abstract!
  include InterfaceChild
end

class AbstractBadChild < Abstract # error: Missing definition for abstract method `Interface#a_method`
end

class AbstractChild < Abstract
  extend T::Sig
  extend T::Helpers

  abstract!
end

class ConcreteChild < AbstractChild
  def a_method; end
end

module ImplementationInterface
  def a_method; end
end

class IndirectConcrete
  include ImplementationInterface
  include Interface
end

class AbstractSingleton
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def self.abstract_method; end
end

class AbstractSingletonBadChild < AbstractSingleton # error: Missing definition for abstract method
end

class AbstractSingletonGoodChild < AbstractSingleton
  def self.abstract_method; end
end
