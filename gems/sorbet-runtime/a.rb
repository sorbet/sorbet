module Opus::Repro
  class A
    sig {returns(T.class_of(Opus::Repro::B))}
    def self.return_to_bee
      Opus::Repro::B
    end
  end
end
