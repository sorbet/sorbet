# typed: true

class A # parser-error: Hint: this "class" token is not closed before the end of the file
  extend T::Sig

  sig {params(x: A).void}
  def example(x) # parser-error: Hint: this "def" token is not closed before the end of the file
    x.
    # ^ completion: example, ...
    # parser-error: unexpected token "end of file"
