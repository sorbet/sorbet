# typed: true
extend T::Sig

sig {params(blk: T.proc.params(args: Integer).void).void}
def foo(&blk)
end

foo do |(*args)|
  T.reveal_type(args) # error: Revealed type: `Integer`
end
