# typed: false

class A
  def test1
    begin # parser-error: Hint: this "begin" token might not be properly closed
      puts 'inside'
    rescue
    puts 'after'
  end

  def test2
      begin # parser-error: Hint: this "begin" token might not be properly closed
        puts 'inside'
    rescue
    puts 'after'
  end

  def test3
    begin # parser-error: Hint: this "begin" token might not be properly closed
      puts 'inside'
    rescue
      puts 'after'
  end

  def test4
    begin # parser-error: Hint: this "begin" token might not be properly closed
      puts 'inside'
    rescue
    puts 'after'
  end
end # parser-error: unexpected token "end of file"
