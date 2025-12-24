# typed: strict

# This test verifies that attempting to instantiate an abstract class
# produces error 5073 (Resolver::AbstractClassInstantiated) which was
# moved from the CFG builder to the definition_validator.
# The benefit is that this error is now caught earlier in the pipeline,
# during definition validation rather than during CFG construction.

class AbstractParent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(String)}
  def foo; end
end

class ConcreteChild < AbstractParent
  sig {override.returns(String)}
  def foo
    "hello"
  end
end

class Instantiator
  extend T::Sig

  sig {void}
  def create_abstract
    AbstractParent.new # error: Attempt to instantiate abstract class `AbstractParent`
  end

  sig {void}
  def create_concrete
    # This is fine - concrete class can be instantiated
    ConcreteChild.new
  end
end

# Test instantiation in various contexts
AbstractParent.new # error: Attempt to instantiate abstract class `AbstractParent`

x = AbstractParent.new # error: Attempt to instantiate abstract class `AbstractParent`

# Test that defining custom .new on abstract class allows instantiation
class AbstractWithCustomNew
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {returns(AbstractWithCustomNew)}
  def self.new
    # Factory method that returns a concrete instance
    ConcreteWithCustomNew.allocate
  end

  sig {abstract.returns(String)}
  def bar; end
end

class ConcreteWithCustomNew < AbstractWithCustomNew
  sig {override.returns(String)}
  def bar
    "custom"
  end
end

# This is fine because AbstractWithCustomNew defines its own .new method
AbstractWithCustomNew.new
