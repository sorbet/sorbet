# typed: false

class A
  def test0
    x = nil
    if
    puts 'after'
  end

  def test1
    x = nil
    if x
    puts 'after'
  end

  def test2
    x = nil
    if x.
    puts 'after'
  end

  def test3
    x = nil
    if x.f
    puts 'after'
  end

  def test4
    x = nil
    if x.f()
    puts 'after'
  end
end
