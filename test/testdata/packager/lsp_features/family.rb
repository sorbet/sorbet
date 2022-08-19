# frozen_string_literal: true
# typed: true

module Simpsons
  #    ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "(nothing)"
  #    ^        show-symbol: Simpsons
  class Family
      # ^^^^^^ def: family
      extend T::Sig

      sig {returns(String)}
      def son_catchphrase
          Bart::CatchPhrase
      #   ^^^^ hover: T.class_of(Bart)
      #   ^    show-symbol: Bart
          #     ^^^^^^^^^^^ usage: catchphrase
          #     ^^^^^^^^^^^ hover: String
          #     ^           show-symbol: Bart::CatchPhrase
        # ^^^^ usage: bartmod
        # ^^^^ importusage: bartpkg
      end

      sig {returns(Bart::Character)}
      #                  ^^^^^^^^^ usage: character
      #            ^^^^ usage: bartmod
      #            ^^^^ importusage: bartpkg
      def son
          Bart::Character.new
          #     ^^^^^^^^^ usage: character
          #     ^^^^^^^^^ hover: T.class_of(Bart::Character)
          #     ^         show-symbol: Bart::Character
          #     ^^^^^^^^^ hover: Character class description
          #               ^^^ hover: Description of Character initialize
        # ^^^^ usage: bartmod
        # ^^^^ importusage: bartpkg
          Bart::C # error: Unable to resolve constant `C`
          #      ^ completion: CatchPhrase, Character
        # ^^^^ usage: bartmod
        # ^^^^ importusage: bartpkg
      end
  end

  Test::Krabappel::Popquiz
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Test::Krabappel::Popquiz` in non-test file
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Krabappel::Popquiz` is defined in a test namespace
  #                ^^^^^^^ usage: popquiz
# ^^^^^^^^^^^^^^^ importusage: krabappel-pkg

  class Private
    #   ^^^^^^^ def: s-private
  end
end
