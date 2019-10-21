# typed: false

def test_method_without_fun_name(x)
  x.
end # error: unexpected token

def test_method_without_fun_name_plus_before(x)
  before = 1
  x.
end # error: unexpected token

def test_method_without_fun_name_plus_after(x)
  x.
  after = 1
end

def test_method_without_fun_name_before_and_after(x)
  before = 1
  x.
  after = 1
end
