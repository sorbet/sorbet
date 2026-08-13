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

class RealSigStillFlattened
  extend T::Sig

  def outer
    sig { void }
    def inner; end
  end
end

RealSigStillFlattened.new.outer
RealSigStillFlattened.new.inner

class WithoutRuntimeSigStillFlattened
  def outer
    T::Sig::WithoutRuntime.sig { void }
    def inner; end
  end
end

WithoutRuntimeSigStillFlattened.new.outer
WithoutRuntimeSigStillFlattened.new.inner
