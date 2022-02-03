module Opus::Repro
  class C
    sig {params(a: T.nilable(T.class_of(Opus::Repro::A)), b: T.nilable(T.class_of(Opus::Repro::B))).void}
    def self.duplex(a, b)
      p a, b
    end
  end
end
