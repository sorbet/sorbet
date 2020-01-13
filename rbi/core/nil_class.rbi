# typed: __STDLIB_INTERNAL

# The class of the singleton object `nil`.
class NilClass < Object
  # And---Returns `false`. *obj* is always evaluated as it is the argument to a
  # method call---there is no short-circuit evaluation in this case.
  sig do
    params(
        obj: BasicObject,
    )
    .returns(FalseClass)
  end
  def &(obj); end

  # Exclusive Or---If *obj* is `nil` or `false`, returns `false`; otherwise,
  # returns `true`.
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ^(obj); end

  # Returns zero as a rational. The optional argument `eps` is always ignored.
  sig {returns(Rational)}
  def rationalize(); end

  # Always returns an empty array.
  #
  # ```ruby
  # nil.to_a   #=> []
  # ```
  sig {returns([])}
  def to_a(); end

  # Returns zero as a complex.
  sig {returns(Complex)}
  def to_c(); end

  # Always returns zero.
  #
  # ```ruby
  # nil.to_f   #=> 0.0
  # ```
  sig {returns(Float)}
  def to_f(); end

  # Always returns an empty hash.
  #
  # ```ruby
  # nil.to_h   #=> {}
  # ```
  sig {returns(T::Hash[T.untyped, T.untyped])}
  def to_h(); end

  # Returns zero as a rational.
  sig {returns(Rational)}
  def to_r(); end

  # Or---Returns `false` if *obj* is `nil` or `false`; `true` otherwise.
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def |(obj); end

  # Only the object *nil* responds `true` to `nil?`.
  sig {returns(TrueClass)}
  def nil?; end
end
