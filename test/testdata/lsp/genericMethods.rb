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
  #        ^ hover: sig { params(a: T.type_parameter(:A)).returns(T.type_parameter(:A)) }
  #        ^ usage: id
  v2 = foo.id("1")
# ^ hover: String
  #        ^ hover: sig { params(a: T.type_parameter(:A)).returns(T.type_parameter(:A)) }
  #        ^ usage: id
  v3 = Foo.id(1)
# ^ hover: Integer
  #        ^ hover: sig { params(a: T.type_parameter(:A)).returns(T.type_parameter(:A)) }
  #        ^ usage: staticid
  v4 = Foo.id("1")
# ^ hover: String
  #        ^ hover: sig { params(a: T.type_parameter(:A)).returns(T.type_parameter(:A)) }
  #        ^ usage: staticid

  v5 = Foo.block_id do
    #       ^ hover-line: 2 sig do
    #       ^ hover-line: 3   params(
    #       ^ hover-line: 4     blk: T.proc.returns(T.type_parameter(:U))
    #       ^ hover-line: 5   )
    #       ^ hover-line: 6   .returns(T.type_parameter(:U))
    #       ^ hover-line: 7 end
    #       ^ usage: block_id
    "1"
  end
end
