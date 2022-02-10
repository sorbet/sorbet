# typed: false

class A
  def test0
    x = nil
    if # error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test1
    x = nil
    if x # error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test2
    x = nil
    if x. # error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test3
    x = nil
    if x.f # error: Hint: this "if" token might not be properly closed
    puts 'after'
  end

  def test4
    x = nil
    if x.f() # error: Hint: this "if" token might not be properly closed
    puts 'after'
  end
end # error: unexpected token "end of file"
