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
  puts StringAlias # error: Expected `BasicObject` but found `Runtime object representing type:
end
StringAlias = T.type_alias {String}


IntegerAlias = T.type_alias {Integer}
module B
  puts IntegerAlias # error: Expected `BasicObject` but found `Runtime object representing type:
end

# Stress-testing nesting / scope resolution of type aliases

class Outside
  class Inside
    def inside
      puts RootAlias # error: Expected `BasicObject` but found `Runtime object representing type:
      puts OutsideAlias # error: Expected `BasicObject` but found `Runtime object representing type:
      puts InsideAlias # error: Expected `BasicObject` but found `Runtime object representing type:
    end

    InsideAlias = T.type_alias {Symbol}
  end

  def outside
    puts RootAlias # error: Expected `BasicObject` but found `Runtime object representing type:
    puts OutsideAlias # error: Expected `BasicObject` but found `Runtime object representing type:
    puts InsideAlias # error: Unable to resolve constant `InsideAlias`
  end

  OutsideAlias = T.type_alias {String}
end

def main
  puts RootAlias # error: Expected `BasicObject` but found `Runtime object representing type:
  puts OutsideAlias # error: Unable to resolve constant `OutsideAlias`
  puts InsideAlias # error: Unable to resolve constant `InsideAlias`
end

RootAlias = T.type_alias {Integer}
