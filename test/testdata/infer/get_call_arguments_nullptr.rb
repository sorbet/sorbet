# typed: true
extend T::Sig

class CallTakesBlock
  def call(x)
    p x
  end
end

sig {params(blk: CallTakesBlock).void}
def example(&blk)
  yield 42
end

example do |x|
  T.reveal_type(x) # error: `T.untyped`
end
