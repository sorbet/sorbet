# typed: strict

class Bart < PackageSpec
    # ^^^^ def: bartpkg
    export Character
    #      ^^^^^^^^^ usage: character
    export CatchPhrase
    #      ^^^^^^^^^^^ usage: catchphrase
end
