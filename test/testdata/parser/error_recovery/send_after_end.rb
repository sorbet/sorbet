# typed: false

# We don't recover from this code well as we'd like: ideally, this should parse
# the method call to `x.if` inside the `def foo`, but instead it puts the
# method call outside the MethodDef node.
#
# This is part of a larger problem that a single mismatched `end` can really
# screw over our ability to recover (see error_recovery_if_no_end.rb).
#
# This code is empirically rare (doesn't accur at all at Stripe), but it's
# unclear how often code like this comes up when people are making edits.

def foo
  x.end

  x.
    end # error: unexpected token

  x.if
end # error: unexpected token
