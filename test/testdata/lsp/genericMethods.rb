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

end

def main
  foo = Foo.new
  v1 = foo.id(1)
# ^ hover: Integer(1)
         # ^ hover: sig {params(a: Integer(1)).returns(Integer(1))}
         # ^ usage: id
  v2 = foo.id("1")
# ^ hover: String("1")
         # ^ hover: sig {params(a: String("1")).returns(String("1"))}
         # ^ usage: id
  v3 = Foo.id(1)
# ^ hover: Integer(1)
         # ^ hover: sig {params(a: Integer(1)).returns(Integer(1))}
         # ^ usage: staticid
  v4 = Foo.id("1")
# ^ hover: String("1")
         # ^ hover: sig {params(a: String("1")).returns(String("1"))}
         # ^ usage: staticid
end
