# typed: strict
module ObjectSpace
  sig {params(object_id: Integer).returns(T.untyped)}
  def self._id2ref(object_id); end

  sig do
    type_parameters(:T)
    .params(mod: T.type_parameter(:T))
    .returns(Enumerator[T.type_parameter(:T)])
  end
  sig do
    type_parameters(:T)
    .params(
      mod: T.type_parameter(:T),
      blk: T.proc.params(obj: T.type_parameter(:T)).void,
    )
    .returns(Integer)
  end
  def self.each_object(mod=Module, &blk)
  end
end
