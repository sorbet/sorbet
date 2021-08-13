# typed: true
extend T::Sig

def do_something
end

def test1
  raise

  if T.unsafe(nil)
  #  ^ error: This code is unreachable
    do_something
  end

  do_something
end

sig {params(x: T.any(Integer, String)).void}
def test2(x)
  case x
  when Integer then puts 'int'
  when String then puts 'str'
  else
    T.absurd(x)
    do_something
  # ^^^^^^^^^^^^ error: This code is unreachable
  end

  do_something
end

sig {params(x: T.any(Integer, String)).void}
def test3(x)
  case x
  when Integer then puts 'int'
  when String then puts 'str'
  else
    T.absurd(x)

    if T.unsafe(nil)
    #  ^^^^^^^^^^^^^ error: This code is unreachable
      do_something
    end
    do_something
  end
end
