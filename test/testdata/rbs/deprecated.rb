# typed: true
# enable-experimental-rbs-comments: true

# Basic deprecated method
class BasicDeprecated
  extend T::Sig
  
  # @deprecated
  #: -> String
  def old_method
    "legacy"
  end
  
  #: -> String
  def new_method
    "current"
  end
end

BasicDeprecated.new.old_method # error: Method `BasicDeprecated#old_method` is deprecated
BasicDeprecated.new.new_method

class DeprecatedVoid
  extend T::Sig
  
  # @deprecated
  #: -> void
  def old_method
  end
end

DeprecatedVoid.new.old_method # error: Method `DeprecatedVoid#old_method` is deprecated

# Deprecated with parameters
class DeprecatedWithParams
  extend T::Sig
  
  # @deprecated
  #: (String x, Integer y) -> String
  def old_method(x, y)
    "#{x}#{y}"
  end
end

DeprecatedWithParams.new.old_method("test", 42) # error: Method `DeprecatedWithParams#old_method` is deprecated

# Deprecated class method
class DeprecatedClassMethod
  extend T::Sig
  
  # @deprecated
  #: -> String
  def self.old_method
    "legacy class method"
  end
end

DeprecatedClassMethod.old_method # error: Method `DeprecatedClassMethod.old_method` is deprecated

# Deprecated with inheritance - deprecated method in parent
class ParentWithDeprecated
  extend T::Sig
  
  # @deprecated
  #: -> String
  def inherited_deprecated
    "parent deprecated"
  end
  
  # @overridable
  # @deprecated
  #: -> String
  def overridable_deprecated
    "parent overridable deprecated"
  end
end

class ChildInheritsDeprecated < ParentWithDeprecated
  extend T::Sig
  
  # Child overrides deprecated method - should still be deprecated
  # @override
  # @deprecated
  #: -> String
  def overridable_deprecated
    "child overrides deprecated"
  end
end

ChildInheritsDeprecated.new.inherited_deprecated # error: Method `ParentWithDeprecated#inherited_deprecated` is deprecated
ChildInheritsDeprecated.new.overridable_deprecated # error: Method `ChildInheritsDeprecated#overridable_deprecated` is deprecated

class ChildMissingInheritedDeprecated < ParentWithDeprecated
  extend T::Sig
  
  # Child overrides deprecated method - should still be deprecated
  # @override
  #: -> String
  def overridable_deprecated # error: Method `ChildMissingInheritedDeprecated#overridable_deprecated` overrides deprecated method but is not marked deprecated
    "child overrides deprecated"
  end
end

ChildMissingInheritedDeprecated.new.inherited_deprecated # error: Method `ParentWithDeprecated#inherited_deprecated` is deprecated
ChildMissingInheritedDeprecated.new.overridable_deprecated

# Deprecated with other annotations
class DeprecatedCombinations
  extend T::Sig
  extend T::Helpers
  abstract!
  
  # @overridable
  # @deprecated
  #: -> String
  def deprecated_and_overridable
    "deprecated but overridable"
  end
  
  # Can be both deprecated and abstract
  sig { deprecated.abstract.void }
  def deprecated_abstract; end
  
  # Can't be both deprecated and final (if we add validation)
  # @final
  # @deprecated
  #: -> void
  def deprecated_final; end
end

class ConcreteDeprecatedCombinations < DeprecatedCombinations
  extend T::Sig
  
  # @override
  #: -> String
  def deprecated_abstract # error: Method `ConcreteDeprecatedCombinations#deprecated_abstract` overrides deprecated method but is not marked deprecated
    "concrete implementation"
  end
end

ConcreteDeprecatedCombinations.new.deprecated_and_overridable # error: Method `DeprecatedCombinations#deprecated_and_overridable` is deprecated
ConcreteDeprecatedCombinations.new.deprecated_abstract
ConcreteDeprecatedCombinations.new.deprecated_final # error: Method `DeprecatedCombinations#deprecated_final` is deprecated

# Test deprecated with attr_accessor-style methods
class DeprecatedAccessors
  extend T::Sig
  
  # @deprecated
  #: String?
  attr_reader :old_attr
  
  # @deprecated
  #: String
  attr_writer :old_attr
end
accessor_instance = DeprecatedAccessors.new
accessor_instance.old_attr # error: Method `DeprecatedAccessors#old_attr` is deprecated
accessor_instance.old_attr = "test" # error: Method `DeprecatedAccessors#old_attr=` is deprecated
