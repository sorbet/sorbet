# typed: strict

class Parent
  sig do
    type_parameters(:U)
      .params(
        blk: T.proc.void
      )
        .returns(Integer)
  end
  def self.foo(&blk)
    yield
    0
  end
end
