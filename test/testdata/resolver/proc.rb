# @typed

class TestProc
  sig(blk: T::Proc[[Integer], returns: Integer])
  .returns(Integer)
  def good1(&blk)
    0
  end

  sig(blk: T::Proc[[T::Array[String]], returns: T::Array[Integer]])
  .returns(Integer)
  def good2(&blk)
    0
  end

  sig(
    x: T::Proc[], # error: Malformed T::Proc[]: Expected a return type
    y: T::Proc[0], # error: Malformed type declaration. Expected `returns: <type>`
    z: T::Proc[[Integer, String]], # error: Malformed type declaration. Expected `returns: <type>`
    z: T::Proc[lemons, returns: Integer], # error: Malformed type declaration. Expected parameter list
  ).returns(NilClass)
  def bad(x, y, z)
  end
end
