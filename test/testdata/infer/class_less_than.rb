# typed: true

extend T::Sig

class Parent; end
class Child < Parent; end

sig {params(x: T.class_of(Parent)).void}
def parent_less_than(x)
  if x < Parent
    T.reveal_type(x) # error: `T.class_of(Parent)`
  else
    T.reveal_type(x) # error: `T.class_of(Parent)`
  end

  if x < Child
    T.reveal_type(x) # error: `T.class_of(Child)`
  else
    T.reveal_type(x) # error: `T.class_of(Parent)`
  end
end

sig {params(x: T.class_of(Parent)).void}
def parent_less_than_eq(x)
  if x <= Parent
    T.reveal_type(x) # error: `T.class_of(Parent)`
  else
    T.reveal_type(x) # error: This code is unreachable
  end

  if x <= Child
    T.reveal_type(x) # error: `T.class_of(Child)`
  else
    T.reveal_type(x) # error: `T.class_of(Parent)`
  end
end

sig {params(x: T.class_of(Child)).void}
def child_less_than(x)
  if x < Parent
    T.reveal_type(x) # error: `T.class_of(Child)`
  else
    T.reveal_type(x) # error: `T.class_of(Child)`
  end

  if x < Child
    T.reveal_type(x) # error: `T.class_of(Child)`
  else
    T.reveal_type(x) # error: `T.class_of(Child)`
  end
end

sig {params(x: T.class_of(Child)).void}
def child_less_than_eq(x)
  if x <= Parent
    T.reveal_type(x) # error: `T.class_of(Child)`
  else
    T.reveal_type(x) # error: This code is unreachable
  end

  if x <= Child
    T.reveal_type(x) # error: `T.class_of(Child)`
  else
    T.reveal_type(x) # error: This code is unreachable
  end
end
