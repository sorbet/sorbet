# typed: strong
extend T::Sig

sig {params(blk: T.proc.void).void}
def example1(&blk)
end

sig {params(blk: T.proc.returns(Integer)).void}
def example2(&blk)
end

sig do
  type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
end
def example3(&blk)
  yield
end

example1 do
  T.unsafe(nil)
end

example2 do
  T.unsafe(nil)
# ^^^^^^^^^^^^^ error: Value returned from block is `T.untyped`
end

# I think that ideally, we would not report the error inside the block, and
# only report an error when you try to use `res`
res = example3 do
  T.unsafe(nil)
# ^^^^^^^^^^^^^ error: Value returned from block is `T.untyped`
end
T.reveal_type(res) # error: `T.untyped`
