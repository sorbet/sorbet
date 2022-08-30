# typed: true

class Child < Parent
  extend T::Sig

  sig {returns(Integer)}
  def self.x
    T.reveal_type(@x) # error: Revealed type: `Integer`
    @x
  end
end
