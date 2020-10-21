# typed: true

class Family
    # ^^^^^^ def: family
    extend T::Sig

    sig {returns(String)}
    def son_catchphrase
        Bart::CatchPhrase
    #   ^^^^ usage: bart
    #   ^^^^ hover: T.class_of(Simpsons::Bart)
        #     ^^^^^^^^^^^ usage: catchphrase
        #     ^^^^^^^^^^^ hover: String("Don\'t have a cow, man.")
    end

    sig {returns(Bart::Character)}
    #            ^^^^ usage: bart
    #                  ^^^^^^^^^ usage: character
    def son
        Bart::Character.new
    #   ^^^^ usage: bart
        #     ^^^^^^^^^ usage: character
        #     ^^^^^^^^^ hover: T.class_of(Bart::Character)
        #     ^^^^^^^^^ hover: Character class description
        #               ^^^ hover: Description of Character initialize
        Bart::C # error: Unable to resolve constant `C`
        #      ^ completion: CatchPhrase, Character
    #   ^^^^ usage: bart
    end
end
