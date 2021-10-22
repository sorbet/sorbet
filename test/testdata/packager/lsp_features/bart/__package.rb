# frozen_string_literal: true
# typed: strict

# Bart package description
class Bart < PackageSpec
    # ^^^^ def: bartpkg
    # ^^^^ symbol-search: "Bart", name = "Bart", container = "(nothing)"
    # ^ show-symbol: Bart
    export Bart::Character
    #            ^^^^^^^^^ usage: character
    #            ^^^^^^^^^ hover: Character class description
    #      ^^^^^^^^^^^^^^^ symbol-search: "Bart", name = "Bart", container = "<PackageTests>::Bart_Package"
    #            ^ show-symbol: Bart::Character
    #      ^ show-symbol: Bart
    export Bart::CatchPhrase
    #            ^^^^^^^^^^^ usage: catchphrase
    #            ^ show-symbol: Bart::CatchPhrase
end
