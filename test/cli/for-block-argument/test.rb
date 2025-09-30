# typed: strict
extend T::Sig

sig { params(blk: T.proc.void).void }
def takes_block(&blk)
  # ...
end

sig { void }
def example
  if [true, false].sample
    blk = -> (){}
  else
    blk = nil
  end

  takes_block(&blk)

  takes_block &blk

  takes_block(*[], &blk)

  takes_block *[], &blk
end

sig {
  type_parameters(:U)
    .params(x: T.all(NilClass, T.type_parameter(:U)))
    .void
}
def example_with_generics(x)
  takes_block(&x)

  # ^ It's unclear whether we actually want an autocorrect there, because if you accept it, you get this, but the situation is really rare anyways.
  if T.unsafe(nil)
    takes_block(&T.must(x))
  end
end

