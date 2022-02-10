# typed: false

class A
  def test1
    if x.f
    end
  end

  def test2
    puts 'before'
    if x
    puts 'after'
end # error: unexpected token "end of file"
