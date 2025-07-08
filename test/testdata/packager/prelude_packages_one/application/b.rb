# typed: true

module Application
  class B
    extend T::Sig

    sig { returns(Prelude::A) }
    def self.test
      Prelude::A.new
    end
  end
end
