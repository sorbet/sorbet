# typed: true
extend T::Sig

# This test is largely a dupe of resolver/bad_sealed_class, but it can't be
# the same test because that one tests that the error is reported at `typed:
# false` while this one records the current behavior that sealed class
# violations still lead to exhaustiveness errors.

class AbstractParent
  extend T::Helpers

  sealed!
  abstract!
end

class ChildGood1 < AbstractParent; end
class ChildGood2 < AbstractParent; end

sig {params(x: AbstractParent).void}
def foo(x)
  case x
  when ChildGood1
  when ChildGood2
  when ChildBad3
    T.reveal_type(x) # error: Revealed type: `ChildBad3`
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `ChildBad4` wasn't handled
  end
end
