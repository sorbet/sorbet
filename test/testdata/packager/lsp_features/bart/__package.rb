# frozen_string_literal: true
# typed: strict

# Bart package description
class Bart < PackageSpec
    # ^^^^ def: bartpkg
    # ^^^^ symbol-search: "Bart", name = "Bart", container = "(nothing)"
    # ^ show-symbol: Bart
    export Bart::Character
    #      ^^^^ symbol-search: "Bart", name = "Bart", container = "Bart"
    #      ^^^^ def: bart 1 not-def-of-self
    #      ^^^^ usage: bartmod
    #            ^^^^^^^^^ usage: character
    #            ^^^^^^^^^ hover: Character class description
    #      ^^^^^^^^^^^^^^^ symbol-search: "Bart", name = "Bart", container = "<PackageTests>::Bart_Package_Private"
    #            ^ show-symbol: Bart::Character
    #      ^ show-symbol: Bart
    export Bart::CatchPhrase
    #      ^^^^ def: bart 2 not-def-of-self
    #      ^^^^ usage: bartmod
    #            ^^^^^^^^^^^ usage: catchphrase
    #            ^ show-symbol: Bart::CatchPhrase
end
