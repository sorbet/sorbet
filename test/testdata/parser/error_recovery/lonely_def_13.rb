# typed: false

class A
  def self.# error: Hint: this "." token might not be followed by a method name
  end
end # error: unexpected token "end of file"
