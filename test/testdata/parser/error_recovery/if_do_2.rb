# typed: false

class A
  def test1
    puts 'before'
    # TODO(jez) Better error would be to point to the `do` keyword here
    if x.y do
      puts 'then'
    end
    Integer.class
  end

  def test2
    puts 'before'
    # TODO(jez) Better error would be to point to the `do` keyword here
    if x.y do
      puts 'then'
    else # error: else without rescue is useless
      puts 'else'
    end
    Integer.class
  end
end # error: unexpected token "end of file"
