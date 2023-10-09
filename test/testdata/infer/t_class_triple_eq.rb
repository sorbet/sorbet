# typed: true
extend T::Sig

class Parent
end

class Child < Parent
end

sig do
  params(
    obj: Parent,
    class_of: T.class_of(Parent),
    t_class: T::Class[Parent]
  )
    .void
end
def example1(obj, class_of, t_class)
  if obj.is_a?(class_of)
    T.reveal_type(obj) # error: `Parent`
  else
    # Will be reached if `class_of` is `Child`
    T.reveal_type(obj) # error: This code is unreachable
  end

  if obj.is_a?(t_class)
    T.reveal_type(obj) # error: `Parent`
  else
    T.reveal_type(obj) # error: `Parent`
  end

  if class_of.===(obj)
    T.reveal_type(obj) # error: `Parent`
  else
    # Will be reached if `class_of` is `Child`
    T.reveal_type(obj) # error: This code is unreachable
  end

  if t_class.===(obj)
    T.reveal_type(obj) # error: `Parent`
  else
    T.reveal_type(obj) # error: `Parent`
  end
end

example1(Parent.new, Child, Child)


sig do
  params(
    obj: T.any(Integer, String),
    class_of: T.any(T.class_of(Integer), T.class_of(String)),
    t_class: T::Class[T.any(Integer, String)]
  )
    .void
end
def example2(obj, class_of, t_class)
  if obj.is_a?(class_of)
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  else
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  end

  if obj.is_a?(t_class)
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  else
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  end

  if class_of.===(obj)
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  else
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  end

  if t_class.===(obj)
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  else
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  end
end

sig do
  params(
    obj: Object,
    class_of: T.any(T.class_of(Integer), T.class_of(String)),
    t_class: T::Class[T.any(Integer, String)]
  )
    .void
end
def example3(obj, class_of, t_class)
  if obj.is_a?(class_of)
    T.reveal_type(obj) # error: `Object`
  else
    T.reveal_type(obj) # error: `Object`
  end

  if obj.is_a?(t_class)
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  else
    T.reveal_type(obj) # error: `Object`
  end

  if class_of.===(obj)
    T.reveal_type(obj) # error: `Object`
  else
    T.reveal_type(obj) # error: `Object`
  end

  if t_class.===(obj)
    T.reveal_type(obj) # error: `T.any(Integer, String)`
  else
    T.reveal_type(obj) # error: `Object`
  end
end
