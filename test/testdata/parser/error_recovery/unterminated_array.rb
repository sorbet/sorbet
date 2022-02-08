# typed: true

class A
  def f
    [1,
  #   ^ error: unexpected token ","
  # ^ error: unterminated "["
    puts "hi"
  end
  def g
    [
  # ^ error: unterminated "["
  end
  def h
    puts "ho"
  end
end

class B
  def q
    puts "ho"
    [
  # ^ error: unterminated "["
  end
  def r
    puts "hi"
  end
  def s
    puts ([] + [)
             # ^ error: unterminated "["
  end
  def t
    puts "hi"
  end
end
