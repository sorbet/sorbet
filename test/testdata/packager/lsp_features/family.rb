# typed: true

class Family
    # ^^^^^^ def: family
    extend T::Sig

    sig {returns(String)}
    def son_catchphrase
        Bart::CatchPhrase
    #   ^^^^ usage: bart
        #     ^^^^^^^^^^^ usage: catchphrase
    end

    # sig {returns(Bart::Character)}
    #             bart
    #                  character
    def son
        Bart::Character.new
    #   ^^^^ usage: bart
        #     ^^^^^^^^^ usage: character
    end
end
