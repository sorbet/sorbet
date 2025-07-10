# typed: true

module Application::B
  class Foo
    extend T::Sig

    sig { returns(Prelude::A) }
    def self.test
      Prelude::A.new
    end
  end
end
