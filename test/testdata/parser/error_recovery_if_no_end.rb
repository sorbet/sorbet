# typed: false

# We still don't recover from this error as much as we'd like to.
# Notably, we currently make it appear as if `class B` is nested in `class A`,
# when what the user probably meant given the indentation was for there to be
# two unrelated classes at the top level.

class A
  def foo
    if true
  end
end

class B
  def bar
  end
end # error: unexpected token
