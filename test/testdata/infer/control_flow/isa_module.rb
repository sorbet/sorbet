# typed: strict
module Mod; end

def isa_module(m)
  if m.is_a?(Mod)
    T.assert_type!(m, Mod)
  end

  case m
  when Mod
    T.assert_type!(m, Mod)
  end
end
