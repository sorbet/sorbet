# typed: __STDLIB_INTERNAL

# The global value `true` is the only instance of class
# [`TrueClass`](https://docs.ruby-lang.org/en/2.7.0/TrueClass.html) and
# represents a logically true value in boolean expressions. The class provides
# operators allowing `true` to be used in logical expressions.
class TrueClass
  # And---Returns `false` if *obj* is `nil` or `false`, `true` otherwise.
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def &(obj); end
  # Exclusive Or---Returns `true` if *obj* is `nil` or `false`, `false`
  # otherwise.
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj); end
  # Or---Returns `true`. As *obj* is an argument to a method call, it is always
  # evaluated; there is no short-circuit evaluation in this case.
  #
  # ```ruby
  # true |  puts("or")
  # true || puts("logical or")
  # ```
  #
  # *produces:*
  #
  # ```
  # or
  # ```
  sig {params(obj: BasicObject).returns(TrueClass)}
  def |(obj); end
  sig {returns(FalseClass)}
  def !; end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end
end
