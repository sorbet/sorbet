# typed: true

module Application
  class B
    extend T::Sig

    sig { returns(Integer) }
    def self.test
      10
    end
  end
end
