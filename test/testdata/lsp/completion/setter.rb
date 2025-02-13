# typed: true

class A
  attr_accessor :foo
end

def main
  a = A.new
  a.
  # ^ completion: foo, foo=, ...
  # ^ apply-completion: [A] item: 1
end # parser-error: unexpected token "end"
