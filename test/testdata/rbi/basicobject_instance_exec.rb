# typed: true
class A
  extend T::Sig
  sig do
    type_parameters(:U, :V)
    .params(
        args: T.type_parameter(:V),
        blk: T.proc.bind(T.untyped).params(args: T.untyped).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_exec(*args, &blk)
    blk.call(*args) # error: Splats are only supported where the size of the array is known statically
  end
end

T.reveal_type(A.new.instance_exec {4}) # error: Revealed type: `Integer(4)`
A.new.instance_exec {T.reveal_type(self)} # error: Revealed type: `T.untyped`
A.new.instance_exec(3) {|x| T.reveal_type(x)} # error: Revealed type: `T.untyped`
