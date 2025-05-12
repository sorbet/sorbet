# typed: strict
# enable-experimental-rbs-comments: true

class ClassErr1 #: as String # error: Unexpected RBS assertion comment found after `class` declaration
end

class ClassErr2
end #: as String # error: Unexpected RBS assertion comment found after `class` end

class ClassErr3; end #: as String # error: Unexpected RBS assertion comment found after `class` end

class ClassStmt
  42 #: String # error: Argument does not have asserted type `String`
end

class ClassStmts
  42 #: String # error: Argument does not have asserted type `String`
  42 #: String # error: Argument does not have asserted type `String`
end
