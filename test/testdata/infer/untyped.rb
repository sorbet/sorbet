# typed: strong
class Module; include T::Sig; end

class A
  sig {void}
  def foo; end
end

sig {returns(T.untyped)}
def returns_untyped
  A.new
end

sig {params(x: Integer).void}
def takes_integer(x)
end

if T.unsafe(nil)
  #^^^^^^^^^^^^^ error: Conditional branch on `T.untyped`
  x = returns_untyped
else
  x = 0
end

x.foo
# ^^^ error: Call to method `foo` on `T.untyped`

y = T.let(x, A)
y.foo

takes_integer(
  T.unsafe(0)
# ^^^^^^^^^^^ error: Argument passed to parameter `x` is `T.untyped`
)

sig {params(blk: T.proc.void).void}
def example(&blk)
end
f = ->(){}
example(&f)

class Parent
  def foo # error: does not have a `sig`
  end
end

class Child < Parent
  def foo # error: does not have a `sig`
    super
    nil
  end
end
