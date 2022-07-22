# typed: true
extend T::Sig

sig do
  type_parameters(:X, :Y)
    .params(blk: T.proc.params(x: T.type_parameter(:X)).returns(T.type_parameter(:Y)))
    .void
end
def takes_generic_block(&blk); end

sig do
  type_parameters(:X, :Y)
    .params(blk: T.proc.params(x: T.type_parameter(:X)).returns(T.type_parameter(:Y)))
    .void
end
def passes_along_generic_block(&blk)
  T.reveal_type(blk)
  takes_generic_block(&blk)
end
