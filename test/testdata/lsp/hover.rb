# typed: true
class BigFoo; extend T::Sig
    # ^ hover: T.class_of(BigFoo)
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
              # ^ hover: Integer
                   # ^ hover: String
    4 + num1 + num2.to_i
           # ^ hover: sig {params(arg0: Integer).returns(Integer)}
  end

  sig {generated.void}
  def self.baz
  end

  sig {params(arg: String).returns(String)}
  def baz(arg)
    arg + self.class.bar(1, "2").to_s
  end

  sig {params(num: Integer).returns(String)}
  private def quux(num)
            # ^ hover: private sig {params(num: Integer).returns(String)}
    if num < 10
      s = 1
    else
      s = "1"
    end
    s.to_s
  end

  sig {void}
  protected def protected_fun; end;
              # ^ hover: protected sig {void}

  sig { returns([Integer, String]) }
  def self.anotherFunc()
    [1, "hello"]
  end
end

module Mod
end

def main
  BigFoo.bar(10, "hello")
        # ^ hover: sig {generated.params(num1: Integer, num2: String).returns(Integer)}
        # ^ hover: ```ruby
        # Checks that we're sending Markdown.
  BigFoo.baz
        # ^ hover: sig {generated.void}
  l = BigFoo.anotherFunc
# ^ hover: [Integer, String] (2-tuple)

  # Test primitive types
  n = nil
# ^ hover: NilClass
  t = true 
# ^ hover: TrueClass
  f = false 
# ^ hover: FalseClass
  r = //
# ^ hover: Regexp
  s = "hello"
# ^ hover: String("hello")
  i = 1
# ^ hover: Integer(1)
  fl = 1.0
# ^ hover: Float(1.000000)
  sym = :test
# ^ hover: Symbol(:"test")
  Mod
# ^ hover: T.class_of(Mod)
  rational = Rational(2, 3)
# ^ hover: Rational
  range = (-1..-5)
# ^ hover: T::Range[Integer]
  tup = [1, 2]
# ^ hover: [Integer(1), Integer(2)] (2-tuple)
  arr = T.let(tup, T::Array[Integer])
# ^ hover: T::Array[Integer]
  hash = Hash.new("default")
# ^ hover: T::Hash[T.untyped, T.untyped]

  raise "error message"
# ^ hover: sig {params(arg0: String).returns(T.noreturn)}
end
