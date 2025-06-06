# typed: true

class Parent; end
class Child < Pare # error: Hint: this "class" token is not closed
  #           ^^^^ error: Unable to resolve constant
  #               ^ completion: Parent

  # error: unexpected token "end of file"
