# @typed
module NS
  class C1 < Alias # error: Superclasses and mixins must be statically resolved.
  end

  class Dest
  end

  Alias = Dest
end
