# typed: true
# enable-experimental-rbs-comments: true

# Test type aliases in nested contexts
# Type aliases are ONLY allowed directly in class/module/sclass bodies,
# NOT inside if/unless/while/methods/etc.

# Type alias directly in module body - should work
module DirectModuleTypeAlias
  #: type directType = String
end

# Type alias directly in class body - should work
class DirectClassTypeAlias
  #: type directType = Integer
end

# Type alias inside `if` at top level - should error
if RUBY_VERSION >= "3.0"
  #: type topLevelIfType = String
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
end

# Module inside if - type alias in module body works
if RUBY_VERSION >= "3.0"
  module VersionSpecificModule
    #: type versionType = String
  end
end

# Class inside unless - type alias in class body works
unless ENV["SKIP_FEATURE"]
  class FeatureClass
    #: type featureType = Integer
  end
end

# Type alias inside `if` inside module - should error
module TypeAliasInIfInModule
  if RUBY_VERSION >= "3.0"
    #: type conditionalType = String
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
  else
    #: type elseType = Integer
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
  end

  # Type alias outside if but still in module body - should work
  #: type moduleLevelType = Float
end

# Type alias inside method - should error
class MethodTypeAliasError
  def bad_method
    #: type badType = String
#   ^^^^^^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
  end
end

# Type alias inside singleton method - should error
class SingletonMethodTypeAliasError
  def self.bad_singleton_method
    #: type singletonBadType = Integer
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
  end
end
