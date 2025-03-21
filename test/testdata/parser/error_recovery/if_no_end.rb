# typed: false

class A
  def foo
    if true # parser-error: Hint: this "if" token might not be properly closed
  end
end

class B
  def bar
  end
end # parser-error: unexpected token "end of file"
