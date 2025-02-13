# typed: true

# We'd like a better parse for this case than we currently are getting. Specifically it would be nice if it didn't drop
# the 1 from the array, nor drop the sig + method def. If you can get closer to the desired behavior, please feel free
# to update the .exp file for this test.

class A
  X = [1,
    # ^ parser-error: unterminated "["
  sig {void}
  def bar
# ^^^ parser-error: unexpected token "def"
  end
end # parser-error: unexpected token "end"
