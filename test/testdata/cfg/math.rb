# typed: true
class Example
  extend T::Sig
  extend T::CFGExport

  sig {params(is_add: T::Boolean, a: Integer, b: Integer).returns(Integer)}
  def self.calculate(is_add, a, b)
    if is_add
      a + b
    else
      a - b
    end
  end
end
