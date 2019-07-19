# typed: strict

module StdlibTest
  extend T::Sig

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
