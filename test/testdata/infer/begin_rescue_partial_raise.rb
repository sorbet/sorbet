# typed: strict
extend T::Sig

# This test is meant to stress the limit of our fancy "merge environments even
# if they're dead."
#
# Normally that's a bad idea, as it make Sorbet think that the types of things
# available in those dead environments still have that type. But since the
# block finished dead, Sorbet's flow-sensitive inference should be powerful
# enough to know that any variables and/or types from that (now-dead)
# environment cannot flow into the current block.
#
# Concretely, Sorbet knows that `x` is `NilClass` in the first `reveal_type`
# below, because it would only have been set in an environment that was dead.
#
# The discovery of this test forced us to complicate `CFGBuilder::simplify`
# slightly more than initially hoped.
#
# And even still, Sorbet's behavior on this isn't as good as we might want,
# because the type of `x` in the `rescue` is not `T.nilable(Integer)` like it
# should be.

sig {void}
def example1
  begin
    if T.unsafe(nil)
      x = 1
      raise
    end
    T.reveal_type(x) # error: `NilClass`
  rescue
    T.reveal_type(x) # error: `T.nilable(Integer)`
  end
end
