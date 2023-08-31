# typed: strict

class Parent
  sig do
    type_parameters(:U)
      .params(
        blk: T.proc.void
      )
        .void
  end
  def initialize(&blk)
    yield
  end
end
