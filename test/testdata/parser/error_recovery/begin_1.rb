# typed: false

class A
  def test1
    begin # error: Hint: this "begin" token might not be properly closed
      puts 'inside'
    rescue
    puts 'after'
  end

  def test2
      begin # error: Hint: this "begin" token might not be properly closed
        puts 'inside'
    rescue
    puts 'after'
  end

  def test3
    begin # error: Hint: this "begin" token might not be properly closed
      puts 'inside'
    rescue
      puts 'after'
  end

  def test4
    begin # error: Hint: this "begin" token might not be properly closed
      puts 'inside'
    rescue
    puts 'after'
  end
end # error: unexpected token "end of file"
