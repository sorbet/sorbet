# typed: strict

class Module
  include T::Sig
end

class Child < Parent
  sig {params(x: Integer).void}
  def self.x=(x)
    @@x = x
  end

  # Ensures that findMemberTransitive correctly looks for @@x,
  # even if classes are defined out of order.

  sig {void}
  def child_foo
    T.reveal_type(@@x) # error: Revealed type: `Integer`
  end

  sig {void}
  def self.child_bar
    T.reveal_type(@@x) # error: Revealed type: `Integer`
  end
end

class Parent
  @@x = T.let(1, Integer)

  sig {void}
  def foo
    puts T.reveal_type(@@x) # error: Revealed type: `Integer`
  end

  sig {void}
  def self.bar
    puts T.reveal_type(@@x) # error: Revealed type: `Integer`
  end
end

# --- useful for demonstrating how @@x works in the runtime ---

Parent.new.foo   # => 1
Parent.bar       # => 1

Child.x = 2      # setting on Child affects parent

Parent.new.foo   # => 2
Parent.bar       # => 2
