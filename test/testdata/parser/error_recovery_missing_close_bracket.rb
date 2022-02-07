# typed: true

class A
  def f
    [1,
    # ^ error: unexpected token ","
    puts "hi"
  end
  def g
    [
  end
# ^^^ error: unexpected token "end"
  def h
    puts "ho"
  end
end

class B
  def q
    puts "ho"
    [
  end # error: unexpected token "end"
  def r
    puts "hi"
  end
  def s
    puts ([] + [)
              # ^ error: unexpected token ")"
  end
  def t
    puts "hi"
  end
end
