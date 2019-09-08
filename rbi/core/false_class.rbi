# typed: __STDLIB_INTERNAL

# The global value `false` is the only instance of class `FalseClass` and
# represents a logically false value in boolean expressions. The class provides
# operators allowing `false` to participate correctly in logical expressions.
class FalseClass
  # And---Returns `false`. *obj* is always evaluated as it is the argument to a
  # method call---there is no short-circuit evaluation in this case.
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj); end

  # Exclusive Or---If *obj* is `nil` or `false`, returns `false`; otherwise,
  # returns `true`.
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj); end

  # Or---Returns `false` if *obj* is `nil` or `false`; `true` otherwise.
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def |(obj)
  end

  sig {returns(TrueClass)}
  def !
  end
end
