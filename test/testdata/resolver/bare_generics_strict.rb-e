# typed: strict

module StdlibTest
  extend T::Sig

  # These are not errors at typed: true, but they are at typed: strict
  sig do
    params(
      a: Array, # error: Generic class without type arguments
      b: Hash, # error: Generic class without type arguments
      c: Set, # error: Generic class without type arguments
      d: Range, # error: Generic class without type arguments
      e: Enumerable, # error: Generic class without type arguments
      f: Enumerator, # error: Generic class without type arguments
    )
    .void
  end
  def foo(a, b, c, d, e, f); end
end
