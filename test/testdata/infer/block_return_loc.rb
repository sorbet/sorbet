# typed: true
extend T::Sig

sig {params(blk: T.proc.returns(String)).void}
def example(&blk)
end

 example {0}
#^^^^^^^^^^^ error: Expected `String` but found `Integer(0)` for block result type
