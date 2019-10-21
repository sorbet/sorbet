# typed: true

class A
  def foo; end
end

def test_1
  A.new
end

def test_2
  x = 1
  x
end

def test_3
  A
end
