# typed: true
class A
  extend T::Sig
  sig do
    type_parameters(:U)
    .params(
        blk: T.proc.bind(T.untyped).params(arg0: T.untyped).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_eval(&blk)
    blk.call(self)
  end
end

T.reveal_type(A.new.instance_eval {4}) # error: Revealed type: `Integer`
A.new.instance_eval {T.reveal_type(self)} # error: Revealed type: `T.untyped`
A.new.instance_eval {|x| T.reveal_type(x)} # error: Revealed type: `T.untyped`
