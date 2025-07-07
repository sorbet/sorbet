# typed: strict
# enable-experimental-rbs-comments: true

#: -> void
def defErr1 #: as String # error: Unexpected RBS assertion comment found after `method` declaration
end

#: -> void
def defErr2
end #: as String # error: Unexpected RBS assertion comment found after `method` end

#: -> void
def defErr3; end #: as String # error: Unexpected RBS assertion comment found after `method` end

#: -> void
def defStmt
  42 #: String # error: Argument does not have asserted type `String`
end

#: -> void
def defStmts
  42 #: String # error: Argument does not have asserted type `String`
  42 #: String # error: Argument does not have asserted type `String`
end

#: (String?) -> String
def defImplicitReturn1(x)
  x #: as String
end

#: (String?) -> String
def defImplicitReturn2(x)
  puts x
  x #: as String
end

#: (String?) -> String
def defReturn1(x)
  return x #: as String
end

#: (String?) -> void
def defReturn2(x)
  return #: as String # error: Unexpected RBS assertion comment found after `return`
end

#: (String?) -> void
def defReturn3(x)
  return begin
    x #: as String
  end
end

#: (String?, String?) -> void
def defReturn4(x, y)
  return begin
    x #: as String
    y #: as String
  end
end

#: (String?) -> void
def defReturn5(x)
  return x, 42 #: as [String, Integer]
end

#: -> void
public def defModifier
  nil #: Integer?
end
