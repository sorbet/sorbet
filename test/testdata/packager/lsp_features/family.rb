# frozen_string_literal: true
# typed: true

module Simpsons
  #    ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "Simpsons"
  #    ^        show-symbol: Simpsons
  class Family
      # ^^^^^^ def: family
      extend T::Sig

      sig {returns(String)}
      def son_catchphrase
          Bart::CatchPhrase
      #   ^^^^ usage: bart
      #   ^^^^ hover: T.class_of(Bart)
      #   ^    show-symbol: Bart
          #     ^^^^^^^^^^^ usage: catchphrase
          #     ^^^^^^^^^^^ hover: String("Don\'t have a cow, man.")
          #     ^           show-symbol: Bart::CatchPhrase
      end

      sig {returns(Bart::Character)}
      #            ^^^^ usage: bart
      #                  ^^^^^^^^^ usage: character
      def son
          Bart::Character.new
      #   ^^^^ usage: bart
          #     ^^^^^^^^^ usage: character
          #     ^^^^^^^^^ hover: T.class_of(Bart::Character)
          #     ^         show-symbol: Bart::Character
          #     ^^^^^^^^^ hover: Character class description
          #               ^^^ hover: Description of Character initialize
          Bart::C # error: Unable to resolve constant `C`
          #      ^ completion: CatchPhrase, Character
      #   ^^^^ usage: bart
      end
  end

  Test::Krabappel::Popquiz # error: Unable to resolve constant `Test`

  class Private
    #   ^^^^^^^ def: s-private
  end
end
