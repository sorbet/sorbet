# typed: strong
extend T::Sig

sig do
  type_parameters(:Return)
    .params(blk: T.proc.returns(T.type_parameter(:Return)))
    .returns(T.type_parameter(:Return))
end
def with_block(&blk)
  yield
end

sig { params(blk: T.proc.returns(T.untyped)).returns(T.untyped) }
def with_block_untyped(&blk)
  yield
end

sig { params(x: Integer, y: String).returns(T.untyped) }
def returns_untyped(x, y)
end

# ---

sig { returns(Integer) }
def example1
  with_block_untyped do
# ^^^^^^^^^^^^^^^^^^^^^ error: Value returned from method is `T.untyped`
    0
  end
end

sig { returns(Integer) }
def example2
  returns_untyped(
# ^^^^^^^^^^^^^^^^ error: Value returned from method is `T.untyped`
    0,
    ''
  )
end
