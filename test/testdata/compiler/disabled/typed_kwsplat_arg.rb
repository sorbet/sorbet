# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

# The runtime asserts that the values of the kwsplat hash are the declared type.
# The compiler, at the time this testcase was written, doesn't do that.
sig {params(msg: String, kwargs: Integer).void}
def foo(msg, **kwargs)
  p msg
end

begin
  foo("shouldn't execute the body", a: 1, b: 2, c: T.unsafe('nope'))
rescue
  p "got exception as expected"
else
  p "we shouldn't be here"
end

foo("should execute the body", a: 1, b: 2, c: 3)
