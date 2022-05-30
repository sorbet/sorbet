# typed: true

class A # error: Hint: this "class" token is not closed before the end of the file
  extend T::Sig

  sig {params(x: Integer).void}
  def foo(x) # error: Hint: this "def" token is not closed before the end of the file
    puts(x)

  # We get a double-reported error here because of the naive/greedy
  # backtracking that we do to recover. The `def bar` is parsed, then we
  # attempt to parse the `def foo`, end up backtracking to just after the
  # puts(x), and then we parse the `def bar` a second time.
  #
  # Reporting an error twice seems less bad than failing to parse entirely.
  sig {void}
  def bar
# ^^^ error: Hint: this "def" token is not closed before the end of the file
# ^^^ error: Hint: this "def" token is not closed before the end of the file

  puts 'after' # error: unexpected token "end of file"
