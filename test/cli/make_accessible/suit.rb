# typed: true
module Chalk
  module Tools
    module Accessible
      def make_accessible(values); end
    end
  end
end

class Suit
  extend Chalk::Tools::Accessible
  VALUES = %w{hearts spades clubs diamonds}
  make_accessible(VALUES)
end


Suit.spades
