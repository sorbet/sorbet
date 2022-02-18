# typed: true

module Family

  class Simpsons
    extend T::Sig

    sig {returns(Bart::Character)}
    def bart
      Bart::Character.new
    end
  end

end
