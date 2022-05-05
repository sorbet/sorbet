# frozen_string_literal: true
# typed: strict

module Bart
  #    ^^^^ def: bartmod
  #    ^^^^ symbol-search: "Bart", name = "Bart", container = "(nothing)"
      CatchPhrase = "Don't have a cow, man."
  #   ^^^^^^^^^^^ def: catchphrase
  #   ^^^^^^^^^^^ symbol-search: "Catchphrase", name = "Bart::CatchPhrase", container = "Bart"

  # Character class description
  class Character
      # ^^^^^^^^^ def: character
      # ^^^^^^^^^ symbol-search: "Character", name = "Bart::Character", container = "Bart"
      extend T::Sig

      # Description of Character initialize
      sig {void}
      def initialize
      end
  end
end
