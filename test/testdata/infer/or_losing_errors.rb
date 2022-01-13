# typed: true

extend T::Sig

class A
  def foo; true; end
end

class B; end

class C; end

class D
  def foo; true; end
end

sig {params(x: T.any(B, A, D)).returns(T::Boolean)}
def bar_beginning(x)
  x.foo
  # ^^^ error: Method `foo` does not exist on `B` component of `T.any(B, A, D)`
end

sig {params(x: T.any(A, B, D)).returns(T::Boolean)}
def bar_middle(x)
  x.foo
  # ^^^ error: Method `foo` does not exist on `B` component of `T.any(A, B, D)`
end

sig {params(x: T.any(A, D, B)).returns(T::Boolean)}
def bar_end(x)
  x.foo
  # ^^^ error: Method `foo` does not exist on `B` component of `T.any(A, D, B)`
end

sig {params(x: T.any(A, B, C, D)).returns(T::Boolean)}
def bar_more_than_one(x)
  x.foo
  # ^^^ error: Method `foo` does not exist on `B` component of `T.any(A, B, C, D)`
  # ^^^ error: Method `foo` does not exist on `C` component of `T.any(A, B, C, D)`
end
