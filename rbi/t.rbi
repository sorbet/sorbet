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

  # These are implemented in C++ when they appear in type context; We
  # implement them here in Ruby so they also exist if they are called
  # in value context. Several of these methods additionally have a C++
  # implementation filled in value context; In that case the prototype
  # here still applies, but additional checking and/or analysis is
  # performed in C++ for that method.
  def self.let(exp, type); end
  def self.assert_type!(exp, type); end
  def self.cast(exp, type); end
  def self.nilable(type); end
  def self.proc(**args); end
  def self.class_of(mod); end
  def self.noreturn; end
  def self.enum(values); end
  def self.untyped; end
  def self.any(arg0, *types); end
  def self.all(arg0, *types); end
  def self.reveal_type(value); end

  sig(arg: T.untyped, error: String).returns(T.untyped)
  def self.must(arg, error=""); end

  def self.coerce(type); end
end

module T::Helpers
end
module T::Generic
  include T::Helpers

  def [](*types); end
end

class T::Array
  def self.[](*types); end
end
class T::Hash
  def self.[](*types); end
end
class T::Set
  def self.[](*types); end
end
class T::Range
  def self.[](*types); end
end
class T::Enumerable
  def self.[](*types); end
end
