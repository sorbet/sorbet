# typed: false

class A
  def test0
    x = nil
    if # parser-error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test1
    x = nil
    if x # parser-error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test2
    x = nil
    if x. # parser-error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test3
    x = nil
    if x.f # parser-error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test4
    x = nil
    if x.f() # parser-error: Hint: this "if" token might not be properly closed
    puts 'after'
  end
end # parser-error: unexpected token "end of file"
