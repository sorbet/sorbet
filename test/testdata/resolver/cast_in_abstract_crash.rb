# typed: true
class A
  extend T::Sig

  sig {abstract.params(blk: T.proc.returns(Out)).void}
  def foo
    @z ||= T.let(nil, T.nilable(T.type_parameter(:U)))
  end
end
