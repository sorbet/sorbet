# typed: true

class Parent
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.make
    new
  end

  sig {params(x: T.attached_class).void} # A bad type definition for `x`
  private_class_method def self.consume(x)
    puts "consumed"
  end

  def self.example_parent
    self.consume(Parent.new)
    #            ^^^^^^^^^^ error: Expected `T.attached_class (of Parent)` but found `Parent` for argument `x`
    self.consume(Parent.make)
    #            ^^^^^^^^^^^ error: Expected `T.attached_class (of Parent)` but found `Parent` for argument `x`
    self.consume(Child.new)
    #            ^^^^^^^^^ error: Expected `T.attached_class (of Parent)` but found `Child` for argument `x`
    self.consume(Child.make)
    #            ^^^^^^^^^^ error: Expected `T.attached_class (of Parent)` but found `Child` for argument `x`
    self.consume(self.new)
    self.consume(self.make)
  end
end

class Child < Parent
  extend T::Sig

  sig {void}
  def say_hi
    puts "hi"
  end

  sig {params(x: T.attached_class).void} # A bad type definition for `x`
  private_class_method def self.consume(x)
    x.say_hi
  end

  def self.example_child
    self.consume(Parent.new)
    #            ^^^^^^^^^^ error: Expected `T.attached_class (of Child)` but found `Parent` for argument `x`
    self.consume(Parent.make)
    #            ^^^^^^^^^^^ error: Expected `T.attached_class (of Child)` but found `Parent` for argument `x`
    self.consume(Child.new)
    #            ^^^^^^^^^ error: Expected `T.attached_class (of Child)` but found `Child` for argument `x`
    self.consume(Child.make)
    #            ^^^^^^^^^^ error: Expected `T.attached_class (of Child)` but found `Child` for argument `x`
    self.consume(self.new)
    self.consume(self.make)
  end
end

Parent.consume(Parent.new) # error: Non-private call to private method `consume` on `T.class_of(Parent)`
Child.consume(Parent.new)  # error: Non-private call to private method `consume` on `T.class_of(Child)`
#             ^^^^^^^^^^ error: Expected `Child` but found `Parent` for argument `x`
Parent.example_parent
Child.example_child

class A
  extend T::Sig

  sig {params(cls: T.class_of(Parent)).void}
  def self.consume_parent(cls)
    cls.consume(Parent.make) # error: Non-private call to private method `consume` on `T.class_of(Parent)`
  end
end

A.consume_parent(Parent)
A.consume_parent(Child)
