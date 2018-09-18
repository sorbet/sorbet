# typed: strict
module T::Helpers
  # We could provide a more-complete signature, but these are already
  # parsed in C++, so there's no need to emit errors twice.

  sig(params: T.untyped).returns(Sorbet::Private::Builder)
  def type_parameters(*params); end

  sig(params: T.untyped).returns(Sorbet::Private::Builder)
  def sig(**params); end
end

module T
  extend T::Helpers

  Sorbet.sig(x: T.untyped).returns(T.untyped)
  def self.unsafe(x); end

  # Once we get generic methods, it should return T.nilable(<type>)
  Sorbet.sig(
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

  sig(exp: T.untyped, type: T.untyped).returns(BasicObject)
  def self.let(exp, type); end

  sig(exp: T.untyped, type: T.untyped).returns(BasicObject)
  def self.assert_type!(exp, type); end

  sig(exp: T.untyped, type: T.untyped).returns(BasicObject)
  def self.cast(exp, type); end

  sig(type: T.untyped).returns(BasicObject)
  def self.nilable(type); end

  def self.proc(**args); end
  def self.class_of(mod); end
  def self.noreturn; end
  def self.enum(values); end
  def self.untyped; end

  sig(arg0: T.untyped, types: T.untyped).returns(BasicObject)
  def self.any(arg0, *types); end

  sig(arg0: T.untyped, types: T.untyped).returns(BasicObject)
  def self.all(arg0, *types); end

  def self.reveal_type(value); end
  def self.type_parameter(var); end
  def self.self_type; end
  def self.type_alias(other); end

  Sorbet.sig(arg: T.untyped, error: String).returns(T.untyped)
  def self.must(arg, error=""); end

  def self.coerce(type); end
end

module T::Generic
  include T::Helpers

  def type_member(fixed: nil); end
  def type_template(fixed: nil); end
  def [](*types); end
end

module T::Array
  def self.[](*types); end
end
module T::Hash
  def self.[](*types); end
end
module T::Set
  def self.[](*types); end
end
module T::Range
  def self.[](*types); end
end
module T::Enumerable
  def self.[](*types); end
end
