# typed: true

class A
  extend T::Sig
  extend T::Generic

  Member = type_member
  Template = type_template

  sig {returns(Template)}
  def foo; end

  sig {returns(Member)}
  def self.foo; end
end
