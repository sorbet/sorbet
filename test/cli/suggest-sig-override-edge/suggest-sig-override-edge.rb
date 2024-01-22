# typed: strict

# initialize can be overridden without overridable/override
class ParentInitialize
  extend T::Sig
  sig {void}
  def initialize; end
end
class ChildInitialize < ParentInitialize
  def initialize; end
end

# Weird edge cases in string parsing for adding 'overridable.'
class ParentIsAbstract
  # Every sig here should have `generated.overridable.` added to it.
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def just_void; end

  sig {abstract.returns(NilClass)}
  def just_returns; end

  sig {abstract.params(x: Integer).void}
  def x_to_void(x); end

  sig {abstract.params(x: Integer).returns(NilClass)}
  def x_to_returns(x); end

  sig do
    abstract
    .params(x: Integer)
    .void
  end
  def multiline_x_to_void(x); end

  sig do
    abstract
    .params(x: Integer)
    .returns(NilClass)
  end
  def multiline_x_to_returns(x); end
end
class ChildNeedsOverride < ParentIsAbstract
  def just_void; end

  def just_returns; end

  def x_to_void(x); end

  def x_to_returns(x); end

  def multiline_x_to_void(x); end

  def multiline_x_to_returns(x); end
end

# Doesn't need overridable because parent doesn't have a sig
class DoesntNeedOverridable
  # random words to trip up our hacky text search:
  # void return sig params generated
  def nope; end
end

class ChildOfDoesntNeedOverridable < DoesntNeedOverridable
  def nope; end
end


# Doesn't need generated because parent already has overridable
class DoesntNeedGenerated
  sig {override.overridable.void}
  def already_overridable; end
end

class ChildOfDoesntNeedGenerated < DoesntNeedGenerated
  def already_overridable; end
end

class DSLParent
  # avoid sigging this
  dsl_optional :opt_string, String
end

class DSLChild < DSLParent
  def opt_string; end
end
