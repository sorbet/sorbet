# typed: true
class A
  extend T::Sig
  extend T::Generic

  X = type_member

  # This is an example of the bug that caused an enforce failure in:
  # https://github.com/sorbet/sorbet/issues/206
  sig {params(x: T.all(X, Integer)).void}
  def bar(x); end
end

