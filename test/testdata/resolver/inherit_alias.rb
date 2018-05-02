# typed: strict
module NS
  class C1 < Alias
  end

  class Dest
  end

  Alias = Dest
end
