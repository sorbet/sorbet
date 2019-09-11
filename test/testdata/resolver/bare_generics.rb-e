# typed: true
class Foo
  extend T::Generic
  A = type_member
end

module Test
  extend T::Sig

  sig do
    params(a: Foo) # error: Generic class without type arguments
    .returns(Foo) # error: Generic class without type arguments
  end
  def run(a)
    spec = {
      api_method: api_method.short_name, # error: does not exist on `Test`
    }
    cspec = spec.clone
    cspec[:params] = 1
    a
  end
end

module StdlibTest
  extend T::Sig

  # These are not errors at typed: true, but they are at typed: strict
  sig do
    params(
      a: Array,
      b: Hash,
      c: Set,
      d: Range,
      e: Enumerable,
      f: Enumerator,
    )
    .void
  end
  def foo(a, b, c, d, e, f); end
end
