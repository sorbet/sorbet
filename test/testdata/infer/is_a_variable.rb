# typed: true
extend T::Sig

# At time of writing, this test captures buggy behavior, and is just meant to
# alert the author if their changes affect this behavior.
# https://github.com/sorbet/sorbet/issues/4358

class Parent; end
class Child < Parent; end

MyChild = T.let(Child, T.class_of(Parent))

sig {void}
def isa_local_variables
  parent = Parent.new
  child = T.let(Child, T.class_of(Parent))
  if parent.is_a?(child) # always false at runtime
    T.reveal_type(parent) # error: `Parent`
  else
    T.reveal_type(parent) # error: This code is unreachable
  end
end

sig {void}
def isa_static_field
  parent = Parent.new
  if parent.is_a?(MyChild) # always false at runtime
    T.reveal_type(parent) # error: `Parent`
  else
    T.reveal_type(parent) # error: This code is unreachable
  end
end

sig {void}
def leq_local_variables
  parent = Parent
  child = T.let(Child, T.class_of(Parent))
  if parent <= child # always false at runtime
    T.reveal_type(parent) # error: `T.class_of(Parent)`
  else
    T.reveal_type(parent) # error: This code is unreachable
  end
end

sig {void}
def leq_static_field
  parent = Parent
  if parent <= MyChild # always false at runtime
    T.reveal_type(parent) # error: `T.class_of(Parent)`
  else
    T.reveal_type(parent) # error: This code is unreachable
  end
end

sig {void}
def triple_eq_local_variables
  parent = Parent.new
  child = T.let(Child, T.class_of(Parent))
  if child.===(parent) # always false at runtime
    T.reveal_type(parent) # error: `Parent`
  else
    T.reveal_type(parent) # error: This code is unreachable
  end
end

sig {void}
def triple_eq_static_field
  parent = Parent.new
  if MyChild.===(parent) # always false at runtime
    T.reveal_type(parent) # error: `Parent`
  else
    T.reveal_type(parent) # error: This code is unreachable
  end
end
