# typed: true

class Foo
  extend T::Sig

  sig do
    params(
      a: Integer,
      b: String,
      k1: Integer,
      k2: String,
      block: T.proc.params(x: Integer).returns(String)
    ).returns(String)
  end
  def bar(a, b, k1:, k2:, &block)
    return block.call(k1) if (b.length == a)
    k2
  end

  sig do
    params(
      args: T::Array[T.untyped],
      kargs: T::Hash[T.untyped, T.untyped],
      block: T.untyped
    ).returns(String)
  end
  def foo_orig(*args, **kargs, &block)
    args_lit = [1, "2"]
    hash_lit = {k1: 1, k2: "2"}
    r1 = bar(1, "2", k1: 3, k2: "4") { "" }
    T.reveal_type(r1) # error: Revealed type: `String`
    r2 = bar(*T.unsafe(args), **kargs, &block)
    T.reveal_type(r2) # error: Revealed type: `T.untyped`
    r3 = bar(*T.unsafe(args), **T.unsafe(kargs)) { "" }
    T.reveal_type(r3) # error: Revealed type: `T.untyped`
    r4 = bar(*args_lit, hash_lit, &block)
    T.reveal_type(r4) # error: Revealed type: `String`
  end

  sig do
    params(
      args: T::Array[T.untyped]
    ).returns(String)
  end # error: Unsupported `sig` for argument forwarding syntax
  def foo_fwd(...)
    r1 = T.unsafe(self).bar(...)
    T.reveal_type(r1) # error: Revealed type: `T.untyped`

    r2 = self.bar(...)
    T.reveal_type(r2) # error: Revealed type: `T.untyped`
  end

  sig do
    params(
      args: T.untyped,
      kargs: T::Hash[T.untyped, T.untyped]
    ).void
  end # error: Unsupported `sig` for argument forwarding syntax
  def foo_fwd1(...)
  end

  sig do
    params(
      a: T.untyped,
      k: T::Hash[T.untyped, T.untyped],
      b: T.untyped
    ).void
  end # error: Unsupported `sig` for argument forwarding syntax
  def foo_fwd2(...)
  end

  def foo_fwd3(...)
    puts args # error: Method `args` does not exist on `Foo`
    puts kargs # error: Method `kargs` does not exist on `Foo`
    puts block # error: Method `block` does not exist on `Foo`
  end

  sig do
    params(
      a: T.untyped,
      b: T::Hash[T.untyped, T.untyped],
      c: T.untyped
    ).void
  end # error: Unsupported `sig` for argument forwarding syntax
  def foo_fwd4(a, b, c, ...)
    puts a, b, c
  end

  def foo_fwd5(a, b, c, ...)
    T.reveal_type(a) # error: Revealed type: `T.untyped`
    T.reveal_type(b) # error: Revealed type: `T.untyped`
    T.reveal_type(c) # error: Revealed type: `T.untyped`
    d = self.bar(a, b, c, ...) # error: Splats are only supported where the size of the array is known statically
    T.reveal_type(d) # error: T.untyped
  end
end

Foo.new.foo_fwd
Foo.new.foo_fwd(1, 2)
Foo.new.foo_fwd(k1: 1, k2: 2)
Foo.new.foo_fwd do puts "foo" end
Foo.new.foo_fwd(1, 2, k1: 1, k2: 2) do puts "foo" end

Foo.new.foo_fwd1 { 1}
Foo.new.foo_fwd1(1, 2, 3, {}) { 1 }
Foo.new.foo_fwd1("") { 1 }
Foo.new.foo_fwd1(1, 2, 3, {a: 1}) { 1 }
Foo.new.foo_fwd1(1, 2, 3, {}) { "" }

Foo.new.foo_fwd2 { 1}
Foo.new.foo_fwd2(1, 2, 3, {}) { 1 }
Foo.new.foo_fwd2("") { 1 }
Foo.new.foo_fwd2(1, 2, 3, {a: 1}) { 1 }
Foo.new.foo_fwd2(1, 2, 3, {}) { "" }

Foo.new.foo_fwd3 { 1}
Foo.new.foo_fwd3(1, 2, 3, {}) { 1 }
Foo.new.foo_fwd3("") { 1 }
Foo.new.foo_fwd3(1, 2, 3, {a: 1}) { 1 }
Foo.new.foo_fwd3(1, 2, 3, {}) { "" }
