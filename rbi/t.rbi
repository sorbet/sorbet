# typed: strict
module T
  sig(x: T.untyped).returns(T.untyped)
  def self.unsafe(x); end

  # Once we get generic methods, it should return T.nilable(<type>)
  sig(
    obj: T.untyped,
    type: Class
  ).returns(T.untyped)
  def self.dynamic_cast(obj, type); end

  # These are implemented in C++ when they appear as types; We
  # implement them here in Ruby so they exist if they are called in
  # value context.
  def self.let(exp, type); end
  def self.assert_type!(exp, type); end
  def self.cast(exp, type); end
  def self.nilable(type); end
  def self.proc(**args); end
  def self.class_of(mod); end
  def self.noreturn; end
  def self.enum(values); end

  # These are implmented as C++ intrinsics, so it's important they do
  # not appear in Ruby source lest they shadow C++'s
  # implementation. We should perhaps fix this in the future so that
  # they can take prototypes from Ruby and implementation from C++.

  # def self.untyped; end
  # def self.any(arg0, *types); end
  # def self.all(arg0, *types); end
end

module T::Helpers
end
module T::Generic
  include T::Helpers
end
