# typed: false

class A
  def test1
    if x.f
    puts 'inside if'
  end
    puts 'after if but inside test1'
  end

  def test2
    if x.f
  end
end # error: unexpected token "end of file"
