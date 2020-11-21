# typed: true

class A < T::Struct
  const :foo, Integer
  alias_method :foo_DEPRECATED_BUT_PUBLIC, :foo
  private :foo

  private def private_bar; end
  alias_method :bar, :private_bar
end

# TODO(jez): The errors seen below are what Ruby actually will say, based on runtime ordering.
# Getting this right statically might be tricky. Some ideas:
#   - Give up on visibility tracking for aliases and say they're always public (so we never give a false error)
#   - Maybe: move aliases into namer? Not positive that will fix the problem.

A.new(foo: 0).foo_DEPRECATED_BUT_PUBLIC
A.new(foo: 0).foo # error: Non-private call to private method `A#foo`

A.new(foo: 0).private_bar # error: Non-private call to private method `private_bar`
A.new(foo: 0).bar # error: Non-private call to private method `bar`
