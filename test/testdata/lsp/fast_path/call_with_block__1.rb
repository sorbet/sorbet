# typed: strict

class A
  extend T::Sig

  sig {params(blk: T.proc.params(arg0: Integer).returns(String)).void}
  def self.takes_blk(&blk)
    yield '' # error: Expected `Integer` but found `String("")` for argument `arg0`
  end
end
