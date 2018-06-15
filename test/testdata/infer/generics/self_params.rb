# typed: strict
class Foo
  extend T::Generic
  Self_Type_Member = type_template()
  Not_A_Self_Type = type_member()

  sig(arg: Self_Type_Member).returns(Self_Type_Member)
  def self.bla(arg)
    arg
  end

  sig(arg: Not_A_Self_Type).returns(Not_A_Self_Type) # error: Expression does not have a fully-defined type
  def self.invalid(arg)
    arg
  end
end

class FooChild < Foo # error: should be declared again
  Self_Type_Member = type_template(fixed: String)
end

FooChild.bla("mda").length
FooChild.bla(1) # error: `Integer(1)` doesn't match `String` for argument `arg`

