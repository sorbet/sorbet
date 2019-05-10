# typed: core
module ObjectSpace
  sig {params(object_id: Integer).returns(T.untyped)}
  def self._id2ref(object_id); end

  sig {params(mod: Module).returns(Enumerator[BasicObject])}
  sig {params(mod: Module, blk: T.proc.params(obj: BasicObject).void).returns(Integer)}
  def self.each_object(mod=BasicObject, &blk)
  end
end
