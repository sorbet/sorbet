# typed: true
class Foo
  extend T::Generic
  extend T::Sig

  sig {type_parameters(:A).params(a: T.type_parameter(:A)).returns(T.type_parameter(:A))}
  def id(a)
    # ^ def: id
    a
  end

  sig {type_parameters(:A).params(a: T.type_parameter(:A)).returns(T.type_parameter(:A))}
  def self.id(a)
         # ^ def: staticid
    a
  end

  sig {
    type_parameters(:U)
      .params(blk: T.proc.returns(T.type_parameter(:U)))
      .returns(T.type_parameter(:U))
  }
  def self.block_id(&blk)
    #      ^ def: block_id
    yield
  end
end

def main
  foo = Foo.new
  v1 = foo.id(1)
# ^ hover: Integer
         # ^ hover: sig { params(a: Integer).returns(Integer) }
         # ^ usage: id
  v2 = foo.id("1")
# ^ hover: String
         # ^ hover: sig { params(a: String).returns(String) }
         # ^ usage: id
  v3 = Foo.id(1)
# ^ hover: Integer
         # ^ hover: sig { params(a: Integer).returns(Integer) }
         # ^ usage: staticid
  v4 = Foo.id("1")
# ^ hover: String
         # ^ hover: sig { params(a: String).returns(String) }
         # ^ usage: staticid

  v5 = Foo.block_id do
    #       ^ hover: sig { params(blk: T.proc.returns(String)).returns(String) }
    #       ^ usage: block_id
    "1"
  end
end
