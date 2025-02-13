# typed: false

class A
  def test1
    puts 'before'
    x do # parser-error: Hint: this "do" token might not be properly closed
    puts 'after'
  end

  def test2
    puts 'before'
    x y do # parser-error: Hint: this kDO_BLOCK token might not be properly closed
    puts 'after'
  end

  def test3
    puts 'before'
    -> do # parser-error: Hint: this kDO_LAMBDA token might not be properly closed
    puts 'after'
  end

  def test4
    puts 'before'
    while x do # parser-error: Hint: this "while" token might not be properly closed
    puts 'after'
  end

  def test5
    puts 'before'
    begin # parser-error: Hint: this "begin" token might not be properly closed
    puts 'after'
  end

  def test6
    puts 'before'
    unless nil # parser-error: Hint: this "unless" token might not be properly closed
    puts 'after'
  end

  def test7
    puts 'before'
    while nil # parser-error: Hint: this "while" token might not be properly closed
    puts 'after'
  end

  def test8
    puts 'before'
    until nil # parser-error: Hint: this "until" token might not be properly closed
    puts 'after'
  end

  def test9
    puts 'before'
    unless # parser-error: unexpected token "unless"
  end

  def test10
    puts 'before'
    while # parser-error: unexpected token "while"
  end

  def test11
    puts 'before'
    until # parser-error: unexpected token "until"
  end
end # parser-error: unexpected token "end of file"
