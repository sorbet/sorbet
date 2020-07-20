# typed: strict

# Bart package description
class Bart < PackageSpec
    # ^^^^ def: bartpkg
    # ^^^^ symbol-search: "Bart", name = "Bart", container = "(nothing)"
    export Character
    #      ^^^^^^^^^ usage: character
    #      ^^^^^^^^^ hover: Character class description
    export CatchPhrase
    #      ^^^^^^^^^^^ usage: catchphrase
end
