# typed: strict

# Because we have a type_member
# disable-fast-path: true

class Module
  include T::Sig
end

class A
  sig {void}
  def foo
    @foo = T.let(nil, T.nilable(Integer))
  end
end

class B
  sig {void}
  def self.foo
    @foo = T.let(nil, T.nilable(Integer))
  end
end

class C
  sig {void}
  def foo
    @foo = T.let(0, Integer) # error: The instance variable `@foo` must be declared inside `initialize` or declared nilable
  end
end

class D
  sig {void}
  def self.foo
    @foo = T.let(0, Integer) # error: The singleton instance variable `@foo` must be declared inside the class body or declared nilable
  end
end

# Would like to make this error
class E
  sig {void}
  def foo
    @x = T.let(nil, T.nilable(Integer)) # error: Redeclaring variable `@x` with mismatching type
  end

  sig {void}
  def bar
    @x = T.let(nil, T.nilable(String))
  end
end

class Box
  extend T::Generic

  Elem = type_member

  sig {void}
  def nilable_elem
    @nilable_elem = T.let(nil, T.nilable(Elem))
  end

  sig {void}
  def non_nil_elem
    @non_nil_elem = T.let(T.unsafe(nil), Elem) # error: The instance variable `@non_nil_elem` must be declared inside `initialize` or declared nilable
  end
end

class F
  sig {void}
  def foo
    @foo = T.let(nil, T.nilable(Box[Integer]))
  end
end
