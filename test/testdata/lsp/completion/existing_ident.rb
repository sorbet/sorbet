# typed: true

def test_before_variable(arg, arg1, arg2, arg3)
  arg
  #  ^ completion: arg, arg1, arg2, arg3
end
