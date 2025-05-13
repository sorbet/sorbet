# typed: strict
# enable-experimental-rbs-comments: true

module ModuleErr1 #: as String # error: Unexpected RBS assertion comment found after `module` declaration
end

module ModuleErr2
end #: as String # error: Unexpected RBS assertion comment found after `module` end

module ModuleErr3; end #: as String # error: Unexpected RBS assertion comment found after `module` end

module ModuleStmt
  42 #: String # error: Argument does not have asserted type `String`
end

module ModuleStmts
  42 #: String # error: Argument does not have asserted type `String`
  42 #: String # error: Argument does not have asserted type `String`
end
