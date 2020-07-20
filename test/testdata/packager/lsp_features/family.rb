# typed: true

class Family
    # ^^^^^^ def: family
    extend T::Sig

    sig {returns(String)}
    def son_catchphrase
        Bart::CatchPhrase
    #   ^^^^ usage: bart
    #   ^^^^ hover: T.class_of(Bart)
        #     ^^^^^^^^^^^ usage: catchphrase
        #     ^^^^^^^^^^^ hover: String("Don\'t have a cow, man.")
    end

    # sig {returns(Bart::Character)}
    #             bart
    #                  character
    def son
        Bart::Character.new
    #   ^^^^ usage: bart
        #     ^^^^^^^^^ usage: character
        #     ^^^^^^^^^ hover: T.class_of(Character)
        #     ^^^^^^^^^ hover: Character class description
        #               ^^^ hover: Description of Character initialize
    end
end
