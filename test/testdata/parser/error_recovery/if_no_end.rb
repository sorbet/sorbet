# typed: false

class A
  def foo
    if true # error: Hint: this "if" token might not be properly closed
  end
end

class B
  def bar
  end
end # error: unexpected token "end of file"
