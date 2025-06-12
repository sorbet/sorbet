# typed: true

class Parent
  sig { overridable.void }
  def example; end
end

class Child < Parent
  sig { void }
  def example; end
end

class Module
  include T::Sig
end
