# typed: true

class A
  def f
    [1,
  #   ^ parser-error: unexpected token ","
  # ^ parser-error: unterminated "["
    puts "hi"
  end
  def g
    [
  # ^ parser-error: unterminated "["
  end
  def h
    puts "ho"
  end
end

class B
  def q
    puts "ho"
    [
  # ^ parser-error: unterminated "["
  end
  def r
    puts "hi"
  end
  def s
    puts ([] + [)
             # ^ parser-error: unterminated "["
  end
  def t
    puts "hi"
  end
end
