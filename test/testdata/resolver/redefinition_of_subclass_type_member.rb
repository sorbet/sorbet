# typed: true
# disable-fast-path: true

# This was a fuzzer-generated test file. The specific edge case that
# it ended up uncovering was that `Foo` here has two type members, and
# `Bar` is attempting to redefine one of them as a constant value in a
# way that also makes use of both type members. This rather specific
# situation caused an internal invariant to be broken, where the
# representation of Bar's class contained a vector of one type
# parameter (the inherited V) while resolving the type of `foo` was
# expecting at least two type parameters (the ones corresponding to
# `K` and `V`), resulting in an out-of-bounds memory access.

class Foo
  extend T::Generic
  extend T::Sig

  V = type_member
  K = type_member

  sig {params(k: K, v: V).returns(K)}
  def foo(k, v)
    k
  end
end

class Bar < Foo # error: Type `V` declared by parent `Foo` must be re-declared in `Bar`
  K = Bar[String,String].new.foo('a', 2)
# ^ error: Type variable `K` needs to be declared as a type_member or type_template, not a static-field
  #       ^^^^^^^^^^^^^ error: All type parameters for `Bar` have already been fixed
end
