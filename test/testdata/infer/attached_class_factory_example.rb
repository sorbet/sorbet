# typed: strict

class Module
  include(T::Sig)
end

class Parent
  private_class_method :new

  sig {returns(T.experimental_attached_class)}
  def self.make
    new
  end

  # Always makes a Parent
  sig {returns(Parent)}
  def self.make_parent
    new
  end
end
class Child < Parent;end

sig {params(child: Child).void}
def takes_child(child); end
sig {params(parent: Parent).void}
def takes_parent(parent); end

# ok to use Child::AttachedClass as a Child
takes_child(Child.make)

# ok to use Parent::AttachedClass as a Parent
takes_parent(Parent.make)

# ok to use Child::AttachedClass as a Parent
takes_parent(Child.make)
