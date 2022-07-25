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
  T.reveal_type(blk) # error: `T.proc.params(arg0: T.type_parameter(:X) (of Object#passes_along_generic_block)).returns(T.type_parameter(:Y) (of Object#passes_along_generic_block))`
  takes_generic_block(&blk)
end
