# typed: true
extend T::Sig

sig {
  type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
}
def takes_block(&blk)
  yield
end

sig {params("&": T.proc.returns(Integer)).void}
#           ^^^^ error: Unknown parameter name `&`
def foo(&)
  #     ^ error: Malformed `sig`. Type not specified for parameter `&`
  res = takes_block(&) # error: Expected `T.proc.returns(T.anything)` but found `NilClass` for block argument
  T.reveal_type(res) # error: `T.untyped`
end
