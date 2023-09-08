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
end
