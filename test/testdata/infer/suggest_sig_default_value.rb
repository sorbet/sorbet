# typed: strict

class A
  extend T::Sig

  sig {params(a: Integer).void}
  def takes_int(a); end

  def string_default(x = "hello") # error: does not have a `sig`
    x
  end

  def int_default(x = 42) # error: does not have a `sig`
    x
  end

  def array_default(x = [1, 2, 3]) # error: does not have a `sig`
    x
  end

  def keyword_default(x:, y: "world") # error: does not have a `sig`
    y
  end

  def usage_wins(x = "hello") # error: does not have a `sig`
    takes_int(x)
  end
end
