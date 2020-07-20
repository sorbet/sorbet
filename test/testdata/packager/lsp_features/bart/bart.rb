# typed: strict

    CatchPhrase = "Don't have a cow, man."
#   ^^^^^^^^^^^ def: catchphrase

# Character class description
class Character
    # ^^^^^^^^^ def: character
    extend T::Sig

    # Description of Character initialize
    sig {void}
    def initialize
    end
end
