# typed: true
class BigFoo; extend T::Sig
  class LittleFoo1; extend T::Sig
  sig {params(num: Integer).returns(Integer)}
    def bar(num)
      3 + num
    # ^ hover: Integer(3)
    end
  end
                
  class LittleFoo2; extend T::Sig
    sig {returns(Integer)}
    def bar
      a = BigFoo::LittleFoo1.new
                      # ^ hover: T.class_of(BigFoo::LittleFoo1)
      a.bar(1)
   # ^ hover: null
    # ^ hover: BigFoo::LittleFoo1
      # ^ hover: sig {params(num: Integer).returns(Integer)}
    end
  end
                        
  sig {generated.params(num1: Integer, num2: String).returns(Integer)}
  def self.bar(num1, num2)
    4 + num1 + num2.to_i
  end

  sig {generated.void}
  def self.baz
  end

  sig {params(arg: String).returns(String)}
  def baz(arg)
    arg + self.class.bar(1, "2").to_s
  end
          
  sig {params(num: Integer).returns(String)}
  def quux(num)
    if num < 10
      s = 1
    else
      s = "1"
    end
    s.to_s
  end
end

def main
  BigFoo.bar(10, "hello")
        # ^ hover: sig {generated.params(num1: Integer, num2: String).returns(Integer)}
  BigFoo.baz
        # ^ hover: sig {generated.void}
end
