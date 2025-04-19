# typed: false

class A
  def test1
    puts 'before'
    x.f do # parser-error: Hint: this "do" token might not be properly closed
    puts 'after'
  end

  def test2
    puts 'before'
    x.f() do # parser-error: Hint: this "do" token might not be properly closed
    puts 'after'
  end

  def test3
    puts 'before'
    f() do # parser-error: Hint: this "do" token might not be properly closed
    puts 'after'
  end

  def test4
    puts 'before'
    f() do # parser-error: Hint: this "do" token might not be properly closed
    puts 'after'
  end
end # parser-error: unexpected token "end of file"
