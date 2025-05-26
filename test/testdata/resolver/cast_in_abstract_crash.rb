# typed: true
class A
  extend T::Sig

  sig {abstract.void}
  def foo
    @z ||= T.let(nil, T.nilable(T.type_parameter(:U)))
  end
end
