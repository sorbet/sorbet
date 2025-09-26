# typed: true

module Application
  class B
    extend T::Sig

    sig { returns([Prelude::First::A, Prelude::Second::B]) }
    #                                 ^^^^^^^^^^^^^^^^^^ error: not imported
    def self.test
      [Prelude::First::A.new, Prelude::Second::B.new]
      #                       ^^^^^^^^^^^^^^^^^^ error: not imported
    end
  end
end
