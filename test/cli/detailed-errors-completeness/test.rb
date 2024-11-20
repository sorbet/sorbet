# typed: true

extend T::Sig

sig { params(x: [Integer]).returns([Integer]) }
def foo(x)
  ['']
end

foo([''])

sig { params(x: [Integer]).void }
def bar(x: [1, ''])
end

h = T.let({}, T::Hash[Symbol, [String]])
bar(**h)

sig {params(blk: T.proc.params(x: [Integer]).returns([Integer])).void}
def baz(&blk)
end

p = T.let(T.unsafe(nil), T.proc.params(x: [String]).returns([String]))
baz(&p)

baz do
  ['']
end

class Upper
end

class Middle < Upper
end

class Lower < Middle
end

class A
  extend T::Generic
  extend T::Sig

  def initialize
    @a = T.let([''], [String])
  end

  X = type_member {{upper: [Middle], lower: [Middle] }}

  sig { overridable.params(x: [Integer], y: [Integer], z: [Integer], blk: T.proc.returns([Integer])).returns([Integer]) }
  def foo(x, y:, z: [1], &blk)
    @a = [1]
    [1]
  end

  sig { overridable.params(rest: [Integer]).void }
  def bar(**rest)
  end
end

A[[String]]

class B < A
  extend T::Generic

  X = type_member {{upper: [Upper], lower: [Lower] }}

  sig { override.params(x: [String], y: [String], z: [String], blk: T.proc.returns([String])).returns([String]) }
  def foo(x, y:, z: [''], &blk)
    ['']
  end

  sig { override.params(rest: [String]).void }
  def bar(**rest)
  end
end

a = [1]

loop do
  a = ['']
  break
end

class C
  extend T::Generic

  X = type_member {{ fixed: [Upper] }}
end

class D < C
  extend T::Generic

  X = type_member {{ fixed: [Middle] }}
end

class E
  extend T::Generic

  X = type_member {{upper: [Lower], lower: [Upper] }}
end

xs = {
  foo: [1],
}

xs[:foo] = ['']

T.let([''], [Integer])

b = T.let([0], [Integer])
1.times do
  b = [""]
end
