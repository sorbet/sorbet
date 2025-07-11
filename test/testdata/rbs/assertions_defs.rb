# typed: strict
# enable-experimental-rbs-comments: true

#: -> void
def self.defsErr1 #: as String # error: Unexpected RBS assertion comment found after `method` declaration
end

#: -> void
def self.defsErr2
end #: as String # error: Unexpected RBS assertion comment found after `method` end

#: -> void
def self.defsErr3; end #: as String # error: Unexpected RBS assertion comment found after `method` end

#: -> void
def self.defsStmt
  42 #: String # error: Argument does not have asserted type `String`
end

#: -> void
def self.defsStmts
  42 #: String # error: Argument does not have asserted type `String`
  42 #: String # error: Argument does not have asserted type `String`
end

#: (String?) -> String
def self.defsImplicitReturn1(x)
  x #: as String
end

#: (String?) -> String
def self.defsImplicitReturn2(x)
  puts x
  x #: as String
end

#: (String?) -> String
def self.defsReturn1(x)
  return x #: as String
end

#: (String?) -> void
def self.defsReturn2(x)
  return #: as String # error: Unexpected RBS assertion comment found after `return`
end
