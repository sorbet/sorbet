# typed: strict
module T
  def self.untyped; end

  sig(x: T.untyped).returns(T.untyped)
  def self.unsafe(x); end

  # Once we get generic methods, it should return T.nilable(<type>)
  sig(
    obj: T.untyped,
    type: Class
  ).returns(T.untyped)
  def self.dynamic_cast(obj, type); end

  # Implemented in C++
  def self.let(exp, type); end
  def self.assert_type!(exp, type); end
  def self.cast(exp, type); end
  def self.nilable(type); end
  def self.any(arg0, *types); end
  def self.all(arg0, *types); end
  def self.proc(**args); end

  sig(
    mod: Module
  ).returns(Object)
  def self.class_of(mod); end

  def self.noreturn; end

  sig(
    values: T::Array[Object],
  ).returns(Object)
  def self.enum(values); end
end

module T::Helpers
end
module T::Generic
  include T::Helpers
end
