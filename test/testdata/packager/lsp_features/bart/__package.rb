# typed: strict

# Bart package description
class Bart < PackageSpec
    # ^^^^ def: bartpkg
    export Character
    #      ^^^^^^^^^ usage: character
    #      ^^^^^^^^^ hover: Character class description
    export CatchPhrase
    #      ^^^^^^^^^^^ usage: catchphrase
end
