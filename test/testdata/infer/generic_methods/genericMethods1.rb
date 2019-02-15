# typed: true
class Foo
  extend T::Generic
  extend T::Sig

  sig {type_parameters(:A).params(a: T.type_parameter(:A)).returns(T.type_parameter(:A))}
  def id(a)
    a
  end

  sig {type_parameters(:A).params(a: T.type_parameter(:A)).returns(T.type_parameter(:A))}
  def self.id(a)
    a
  end

end

foo = Foo.new
T.let(foo.id(1), Integer)
T.let(foo.id("1"), String)
T.let(Foo.id(1), Integer)
T.let(Foo.id("1"), String)
