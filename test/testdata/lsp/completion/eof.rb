# typed: true

class A # error: Hint: this "class" token is not closed before the end of the file
  extend T::Sig

  sig {params(x: A).void}
  def example(x) # error: Hint: this "def" token is not closed before the end of the file
    x.
    # ^ completion: example, ...
    # error: unexpected token "end of file"
