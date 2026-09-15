# typed: true
# https://github.com/sorbet/sorbet/issues/7768
#
# The Flatten rewriter pass used to hoist any Send named `sig` out of a method
# body, regardless of receiver, mistaking `x.sig { ... }` for a real Sorbet
# signature. That moved the call to class scope where `x` didn't exist,
# producing bogus type errors. Flatten should only treat a `sig` call as a
# real signature when it's a bare `sig { ... }` (implicit self) or
# `T::Sig::WithoutRuntime.sig { ... }`.

class A
  extend T::Sig

  def foo(x)
    x.sig { void } # error: Method `void` does not exist on `A`
  end
end

class HasSigMethod
  def sig
    "not a signature"
  end
end

class UsesHasSigMethod
  def bar(h)
    T.reveal_type(h.sig) # error: Revealed type: `T.untyped`
  end
end

# A receiver whose constant chain looks superficially similar to
# `T::Sig::WithoutRuntime` (same leaf name and nesting depth) but isn't --
# Flatten must not treat this as a real signature either.
module NotSorbet
  module Sig
    module WithoutRuntime
      def self.sig(&blk); end
    end
  end
end

class UsesFakeWithoutRuntime
  def outer
    NotSorbet::Sig::WithoutRuntime.sig { void } # error: Method `void` does not exist on `UsesFakeWithoutRuntime`
    def inner(x)
      T.reveal_type(x) # error: Revealed type: `T.untyped`
    end
  end
end

# Real signatures must still be found and flattened alongside their nested
# `def`, with the type info actually applying to the resulting method (not
# just the method existing).
class RealSigStillFlattened
  extend T::Sig

  def outer
    sig { params(x: Integer).void }
    def inner(x); end
  end
end

RealSigStillFlattened.new.outer
RealSigStillFlattened.new.inner(1)
RealSigStillFlattened.new.inner("nope") # error: Expected `Integer` but found `String("nope")` for argument `x`

class WithoutRuntimeSigStillFlattened
  def outer
    T::Sig::WithoutRuntime.sig { params(x: Integer).void }
    def inner(x); end
  end
end

WithoutRuntimeSigStillFlattened.new.outer
WithoutRuntimeSigStillFlattened.new.inner(1)
WithoutRuntimeSigStillFlattened.new.inner("nope") # error: Expected `Integer` but found `String("nope")` for argument `x`
