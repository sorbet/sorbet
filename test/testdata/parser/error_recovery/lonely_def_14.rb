# typed: true

# This is an example of where our greedy heuristic can sometimes do the wrong
# thing. If `def bar` had been properly closed, the `def foo` would have parsed
# ok, despite being on a new line.
#
# When we enter recovery mode, we greedily assume that seeing `def` and `foo`
# on different lines must be the cause of the syntax error, which is why the
# snapshot test shows that Sorbet recovers from this parse error by treating
# `foo` as a call in a method with a missing name, instead of as the method
# name itself.
#
# It's tests like this why all the `Hint` errors say "might," because they can
# sometimes be wrong.

class A
  def # error: Hint: this "def" token might not be followed by a method name
    foo # error: Method `foo` does not exist on `A`
  end
  def bar # error: Hint: this "def" token might not be properly closed
end # error: unexpected token "end of file"
