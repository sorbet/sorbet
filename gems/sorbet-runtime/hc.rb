module Opus::Repro
  class HC
    MT = T.type_alias do
      T.any(
        D1,
        I,
        T.class_of(I),
      )
    end

    DT = T.type_alias do
      T.proc.params(
        m: T.nilable(MT)
      ).returns(T.untyped)
    end

    sig {returns(DT)}
    def ddt
      lambda do |m| nil end
    end

    sig {void}
    def initialize
      super
      @d = T.let(ddt, DT)
    end
  end
end
