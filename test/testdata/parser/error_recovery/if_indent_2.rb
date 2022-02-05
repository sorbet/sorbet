# typed: false

class A
  def test0
    x = nil
    if
    puts 'after'
  end # error: Closing "end" token was not indented as far as "if" token

  def test1
    x = nil
    if x
    puts 'after'
  end # error: Closing "end" token was not indented as far as "if" token

  def test2
    x = nil
    if x.
    puts 'after'
  end # error: Closing "end" token was not indented as far as "if" token

  def test3
    x = nil
    if x.f
    puts 'after'
  end # error: Closing "end" token was not indented as far as "if" token

  def test4
    x = nil
    if x.f()
    puts 'after'
  end # error: Closing "end" token was not indented as far as "if" token
end
