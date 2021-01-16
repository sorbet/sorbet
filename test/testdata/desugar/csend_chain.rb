# typed: true

class A
  extend T::Sig

  sig {params(x: T.nilable(String)).void}
  def self.foo(x)
    x&.downcase&.end_with?(x.upcase)
    x&.downcase&.capitalize&.end_with?(x.upcase)
    x&.downcase&.end_with?(x&.upcase) # error: Used `&.` operator on a receiver which can never be nil
  end

  sig {params(y: T::Array[String]).void}
  def self.bar(y)
    y.first&.downcase&.end_with?(y.first.upcase)
                               # ^^^^^^^^^^^^^^ error: Method `upcase` does not exist on `NilClass` component of `T.nilable(String)`
  end
end
