# typed: true

class C
  extend T::Sig

  sig { returns(TrueClass) }
  def !
    true
  end
end

def test1
  if C.new
    puts
  else
    puts # error: This code is unreachable
  end
end

def test2
  if !C.new
    puts
  else
    puts # error: This code is unreachable
  end
end

def test3
  unless C.new
    puts # error: This code is unreachable
  else
    puts
  end
end

def test4
  unless !C.new
    puts # error: This code is unreachable
  else
    puts
  end
end

def test5
  until C.new
    puts # TODO this puts should be unreachable
  end
  puts # error: This code is unreachable
end

def test6
  until !C.new
    # TODO this C.new should be reachable
    puts # error: This code is unreachable
  end
  puts
end
