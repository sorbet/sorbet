# typed: true

class TypedTrue
  def self.implicit_yield
    yield
  end

  def self.explicit_yield(&blk)
    yield
  end

  def self.never_yields
  end
end

# Never an error, because mentions a block arg
TypedTrue.explicit_yield {}
TypedStrict.explicit_yield {}

# Error in this file (`typed: true`) as long as method being called is defined
# in `typed: strict` or above
TypedTrue.never_yields {}
TypedStrict.never_yields {} # error: does not take a block

# Technically this causes problems too, but this should not happen in practice,
# because there will have been an error reported for
# `TypedStrict.implicit_yield`
TypedTrue.implicit_yield {}
TypedStrict.implicit_yield {} # error: does not take a block
