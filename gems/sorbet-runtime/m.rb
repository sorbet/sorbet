module Opus::Repro
  class M < T::Enum
    enums do
      A = new('a')
      B = new('b')
    end

    sig do
      params(
        m: T.nilable(T.class_of(Opus::Repro::D1))
      )
        .returns(T.nilable(M))
    end
    def self.from_thing(m)
      return nil
    end
  end
end
