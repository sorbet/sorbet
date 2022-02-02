# frozen_string_literal: true
# typed: strict

# Bart package description
class Bart < PackageSpec
    # ^^^^ def: bartpkg
    # ^ show-symbol: Bart
    export Bart::Character
    #      ^^^^ usage: bartmod
    #            ^^^^^^^^^ usage: character
    #            ^^^^^^^^^ hover: Character class description
    #            ^ show-symbol: Bart::Character
    export Bart::CatchPhrase
    #      ^^^^ usage: bartmod
    #            ^^^^^^^^^^^ usage: catchphrase
    #            ^ show-symbol: Bart::CatchPhrase
end
