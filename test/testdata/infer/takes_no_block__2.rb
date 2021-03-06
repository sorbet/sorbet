# typed: strict

class TypedStrict
  extend T::Sig

  sig {void}
  def self.implicit_yield
# ^^^^^^^^^^^^^^^^^^^^^^^ error: uses `yield` but does not mention a block parameter
    yield
  end

  sig {params(blk: T.proc.void).void}
  def self.explicit_yield(&blk)
    yield
  end

  sig {void}
  def self.never_yields
  end
end

# Never an error, because mentions a block arg
TypedTrue.explicit_yield {}
TypedStrict.explicit_yield {}

# Error only when defined in `typed: strict`
TypedTrue.never_yields {}
TypedStrict.never_yields {} # error: does not take a block

# Techically this causes problems too, but this should not happen in practice,
# because there will have been an error reported for
# `TypedStrict.implicit_yield`
TypedTrue.implicit_yield {}
TypedStrict.implicit_yield {} # error: does not take a block
