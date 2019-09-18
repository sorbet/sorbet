# typed: true
class A
  extend T::Sig\
  
  sig {params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end

def main
  A.new.bar(10)
       # ^ hover: sig {params(x: Integer).returns(String)}
end

# Tests for https://github.com/sorbet/sorbet/issues/1776
def issue_1776
  [
    # Test hovering over first item in array
    "foo",
    #   ^ hover: String("foo")
    #    ^ hover: String("foo")
    #     ^ hover: null

    # Test hovering over comment
    # ^ hover: null
    
    # Test hovering over last item in array
    123
    # ^ hover: Integer(123)
    #  ^ hover: Integer(123)
    #   ^ hover: null
  ]
end