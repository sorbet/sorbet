# typed: true
# inlay-type-hints: after_var

class ParamHintTest
  extend T::Sig

  sig {params(name: String, age: Integer).returns(String)}
  def self.greet(name, age)
    "Hello #{name}, age #{age}"
  end

  sig {params(x: Integer, y: Integer).returns(Integer)}
  def self.add(x, y)
    x + y
  end

  sig {void}
  def self.main
    ParamHintTest.greet("Alice", 30)
    #                   ^ inlay-hint-param: name:
    #                            ^ inlay-hint-param: age:

    ParamHintTest.add(1, 2)
    #                 ^ inlay-hint-param: x:
    #                    ^ inlay-hint-param: y:
  end
end
