# typed: true

class A
  extend T::Sig

  sig {params(_: Integer, _: Integer).returns(String)} 
                        # ^ error: Parameter name `_` is duplicated
  def self.x(_, _)
    ""
  end
end
