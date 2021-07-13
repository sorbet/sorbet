# typed: true
extend T::Sig

# At time of writing, this test captures buggy behavior, and is just meant to
# alert the author if their changes affect this behavior.
# https://github.com/sorbet/sorbet/issues/4358

class Parent; end
class Child < Parent; end

sig {void}
def leq_local_variables
  parent = T.let(Parent, T.class_of(Parent))
  child = T.let(Child, T.class_of(Parent))
  if parent <= child # always false
    T.reveal_type(parent) # error: `T.class_of(Parent)`
  else
    # [bug] always reached
    T.reveal_type(parent) # error: This code is unreachable
  end
end

MyChild = T.let(Child, T.class_of(Parent))

sig {void}
def leq_static_field
  parent = T.let(Parent, T.class_of(Parent))
  if parent <= MyChild # always false
    T.reveal_type(parent) # error: `T.class_of(Parent)`
  else
    # [bug] always reached
    T.reveal_type(parent) # error: This code is unreachable
  end
end
