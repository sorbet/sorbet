# typed: strict
# enable-experimental-rbs-comments: true

class SClassErr1
  class << self #: as String # error: Unexpected RBS assertion comment found after `sclass` declaration
  end
end

class SClassErr2
  class << self
  end #: as String # error: Unexpected RBS assertion comment found after `sclass` end
end

class SClassErr3
  class << self; end #: as String # error: Unexpected RBS assertion comment found after `sclass` end
end

class SClassStmt
  class << self
    42 #: String # error: Argument does not have asserted type `String`
  end
end

class SClassStmts
  class << self
    42 #: String # error: Argument does not have asserted type `String`
    42 #: String # error: Argument does not have asserted type `String`
  end
end
