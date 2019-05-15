# typed: true
module ObjectSpace
  extend T::Sig
  sig do
    type_parameters(:T)
    .params(mod: T.type_parameter(:T))
    .returns(Enumerator[T.type_parameter(:T)])
  end
  def self.each_object(mod=Module, &blk)
    T.unsafe(nil)
  end
end

T.reveal_type(ObjectSpace.each_object(Integer)) # error: Enumerator[T.class_of(Integer)]
