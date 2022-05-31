# typed: true

module HoldingContainer
  extend T::Sig

  MEMBER_FIELD = T.let("", String)

  sig {returns(String)}
  def self.fancy_function
    "shiny"
  end
end
