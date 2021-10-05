# frozen_string_literal: true
# typed: strict

# Bart package description
class Bart < PackageSpec
    # ^^^^ def: bartpkg
    # ^^^^ symbol-search: "Bart", name = "Bart", container = "(nothing)"
    export Bart::Character
    #            ^^^^^^^^^ usage: character
    #            ^^^^^^^^^ hover: Character class description
    #      ^^^^^^^^^^^^^^^ symbol-search: "Bart", name = "Bart", container = "<PackageTests>::Bart_Package"
    export Bart::CatchPhrase
    #            ^^^^^^^^^^^ usage: catchphrase
end
