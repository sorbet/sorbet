# typed: strict
module ObjectSpace
  sig {params(mod: Module).returns(Enumerator[BasicObject])}
  sig {params(mod: Module, blk: T.proc.params(obj: BasicObject).void).returns(Integer)}
  def self.each_object(mod=BasicObject, &blk)
  end
end
