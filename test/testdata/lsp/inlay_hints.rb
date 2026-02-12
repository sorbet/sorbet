# typed: true
# inlay-type-hints: after_var

class InlayHintTest
  extend T::Sig

  sig {params(name: String, age: Integer).returns(String)}
  def self.greet(name, age)
    "Hello #{name}, age #{age}"
  end

  sig {returns(String)}
  def self.get_name
    "Alice"
  end

  sig {params(x: Integer, y: Integer).returns(Integer)}
  def self.add(x, y)
    x + y
  end

  sig {void}
  def self.main
    result = InlayHintTest.greet("Alice", 30)
    #     ^ inlay-hint: : String

    name = InlayHintTest.get_name
    #   ^ inlay-hint: : String

    sum = InlayHintTest.add(1, 2)
    #  ^ inlay-hint: : Integer
  end
end
