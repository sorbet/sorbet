# typed: strict

# There should be no errors for accessing instance variables here, because the
# Prop rewriter pass declares them.

class AbstractGrandparent < T::InexactStruct
  # Trying to be representative that we won't always see inheritance syntactically.
end

class Parent < AbstractGrandparent
  const :in_parent, Integer

  sig {void}
  def parent_helper
    p @in_parent
  end
end

class Child < Parent
  const :in_child, String

  sig {void}
  def child_helper
    p @in_parent
    p @in_child
  end
end
