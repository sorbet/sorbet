# typed: true

# The ordering between type alias definition / usage shouldn't matter, which is
# why most of the usages are above definitions below (because this ordering is
# most-likely to be buggy)
#
# Also note that we're using scope-based constant resolution here (have to
# search for StringAlias in a different scope from where it's used)

module Main
  extend T::Sig

  sig {params(x: StringAlias, y: IntegerAlias).void}
  def foo(x, y); end
end

module A
  StringAlias
end
StringAlias = T.type_alias(String)


IntegerAlias = T.type_alias(Integer)
module B
  IntegerAlias
end

