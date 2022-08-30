# typed: true

class A
  extend T::Sig

  @x = T.let("", String)

  sig {void}
  def self.example
    T.reveal_type(@x) # error: Revealed type: `String`
  end
end
