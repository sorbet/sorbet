# typed: true

module Application
  class B
    extend T::Sig

    sig { returns([Prelude::First::A, Prelude::Second::B]) }
    def self.test
      [Prelude::First::A.new, Prelude::Second::B.new]
    end
  end
end
