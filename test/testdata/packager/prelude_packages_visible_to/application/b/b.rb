# typed: true

module Application::B
  class Foo
    extend T::Sig

    sig { returns(Prelude::A) }
    #             ^^^^^^^^^^ error: `Prelude::A` resolves but its package is not imported
    def self.test
      Prelude::A.new # error: `Prelude::A` resolves but its package is not imported
    end
  end
end
