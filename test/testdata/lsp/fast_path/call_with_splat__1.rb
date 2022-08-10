# typed: strict

class A
  extend T::Sig

  sig {returns(String)}
  def foo
    ''
  end

  sig do
    type_parameters(:U)
      .params(blk: T.proc.params(arg0: A).returns(T.type_parameter(:U)))
      .returns(T.type_parameter(:U))
  end
  def self.example(&blk)
    yield A.new
  end

  sig {params(x: Integer, y: String).void}
  def self.takes_int_and_string(x, y); end
end
