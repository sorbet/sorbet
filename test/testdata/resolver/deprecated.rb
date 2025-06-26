# typed: true
# enable-deprecated: true

# Basic deprecated method
class BasicDeprecated
  extend T::Sig
  
  sig { deprecated.returns(String) }
  def old_method
    "legacy"
  end
  
  sig { returns(String) }
  def new_method
    "current"
  end
end

BasicDeprecated.new.old_method # hint: Method `BasicDeprecated#old_method` is deprecated
BasicDeprecated.new.new_method

class DeprecatedVoid
  extend T::Sig
  
  sig { deprecated.void }
  def old_method
  end
end

DeprecatedVoid.new.old_method # hint: Method `DeprecatedVoid#old_method` is deprecated

# Deprecated with parameters
class DeprecatedWithParams
  extend T::Sig
  
  sig { deprecated.params(x: String, y: Integer).returns(String) }
  def old_method(x, y)
    "#{x}#{y}"
  end
end

DeprecatedWithParams.new.old_method("test", 42) # hint: Method `DeprecatedWithParams#old_method` is deprecated

# Deprecated class method
class DeprecatedClassMethod
  extend T::Sig
  
  sig { deprecated.returns(String) }
  def self.old_method
    "legacy class method"
  end
end

DeprecatedClassMethod.old_method # hint: Method `DeprecatedClassMethod.old_method` is deprecated

# Deprecated with inheritance - deprecated method in parent
class ParentWithDeprecated
  extend T::Sig
  
  sig { deprecated.returns(String) }
  def inherited_deprecated
    "parent deprecated"
  end
  
  sig { overridable.deprecated.returns(String) }
  def overridable_deprecated
    "parent overridable deprecated"
  end
end

class ChildInheritsDeprecated < ParentWithDeprecated
  extend T::Sig
  
  # Child overrides deprecated method - should still be deprecated
  sig { override.deprecated.returns(String) }
  def overridable_deprecated
    "child overrides deprecated"
  end
end

ChildInheritsDeprecated.new.inherited_deprecated # hint: Method `ParentWithDeprecated#inherited_deprecated` is deprecated
ChildInheritsDeprecated.new.overridable_deprecated # hint: Method `ChildInheritsDeprecated#overridable_deprecated` is deprecated

class ChildMissingInheritedDeprecated < ParentWithDeprecated
  extend T::Sig
  
  # Child overrides deprecated method - should still be deprecated
  sig { override.returns(String) }
  def overridable_deprecated # error: Method `ChildMissingInheritedDeprecated#overridable_deprecated` overrides deprecated method but is not marked deprecated
    "child overrides deprecated"
  end
end

ChildMissingInheritedDeprecated.new.inherited_deprecated # hint: Method `ParentWithDeprecated#inherited_deprecated` is deprecated
ChildMissingInheritedDeprecated.new.overridable_deprecated

# Deprecated with other annotations
class DeprecatedCombinations
  extend T::Sig
  extend T::Helpers
  abstract!
  
  sig { deprecated.overridable.returns(String) }
  def deprecated_and_overridable
    "deprecated but overridable"
  end
  
  # Can be both deprecated and abstract
  sig { deprecated.abstract.void }
  def deprecated_abstract; end
  
  # Can't be both deprecated and final (if we add validation)
  sig(:final) { deprecated.void }
  def deprecated_final; end
end

class ConcreteDeprecatedCombinations < DeprecatedCombinations
  extend T::Sig
  
  sig { override.returns(String) }
  def deprecated_abstract # error: Method `ConcreteDeprecatedCombinations#deprecated_abstract` overrides deprecated method but is not marked deprecated
    "concrete implementation"
  end
end

ConcreteDeprecatedCombinations.new.deprecated_and_overridable # hint: Method `DeprecatedCombinations#deprecated_and_overridable` is deprecated
ConcreteDeprecatedCombinations.new.deprecated_abstract
ConcreteDeprecatedCombinations.new.deprecated_final # hint: Method `DeprecatedCombinations#deprecated_final` is deprecated

# Test deprecated with attr_accessor-style methods
class DeprecatedAccessors
  extend T::Sig
  
  sig { deprecated.returns(T.nilable(String)) }
  attr_reader :old_attr
  
  sig { deprecated.params(old_attr: String).returns(String) }
  attr_writer :old_attr

  sig { deprecated.returns(T.nilable(String)) }
  attr_accessor :accessor_attr
end
accessor_instance = DeprecatedAccessors.new
accessor_instance.old_attr # hint: Method `DeprecatedAccessors#old_attr` is deprecated
accessor_instance.old_attr = "test" # hint: Method `DeprecatedAccessors#old_attr=` is deprecated

accessor_instance.accessor_attr # hint: Method `DeprecatedAccessors#accessor_attr` is deprecated
accessor_instance.accessor_attr = "test" # hint: Method `DeprecatedAccessors#accessor_attr=` is deprecated

