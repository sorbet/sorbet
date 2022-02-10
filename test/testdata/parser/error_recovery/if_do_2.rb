# typed: false

class A
  def test1
    puts 'before'
    # TODO(jez) Better error would be to point to the `do` keyword here
    if x.y do # error: Hint: this "if" token might not be properly closed
      puts 'then'
    end
    Integer.class
  end

  def test2
    puts 'before'
    # TODO(jez) Better error would be to point to the `do` keyword here
    if x.y do # error: Hint: this "if" token might not be properly closed
      puts 'then'
    else # error: else without rescue is useless
      # TODO(jez) better parse would drop the `do` keyword and put this branch in the else
      puts 'else'
    end
    Integer.class
  end
end # error: unexpected token "end of file"
