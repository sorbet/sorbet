# typed: true

extend T::Sig

sig { params(x: Integer, blk: T.nilable(T.proc.void)).void }
def optional_block(x, &blk)
end

sig { params(x: Integer, blk: T.proc.void).void }
def required_block(x, &blk)
end

sig { void }
def no_params
end

optional_block(1)
optional_block(1) {}

required_block(1)
#                ^ error: `required_block` requires a block parameter, but no block was passed
required_block(1) {}

no_params()
