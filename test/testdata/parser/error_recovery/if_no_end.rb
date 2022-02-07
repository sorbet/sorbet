# typed: false

class A
  def foo
    if true
  end # error: Hint: closing "end" token was not indented as far as "if" token
end

class B
  def bar
  end
end # error: unexpected token "end of file"
