# typed: true
extend T::Sig
extend T::Generic

sig do
  type_parameters(:T)
  .params(blk: T.proc.returns(T.type_parameter(:T)))
  .returns(T.type_parameter(:T))
end
def callit(&blk)
  blk.call
end

def test_it
  callit do
    raise

    puts :dead # error: This code is unreachable
  end

  # Test that we are able to propagate from "blk never returns" to
  # "callit never returns"
  puts :dead # error: This code is unreachable
end
