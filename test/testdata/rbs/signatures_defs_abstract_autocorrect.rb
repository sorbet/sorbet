# typed: strict
# enable-experimental-rbs-comments: true

# @abstract
class Abstract
  # @abstract
  #: -> void
  def foo; end # error: Methods declared @abstract with an RBS comment must always raise

  # @abstract
  #: -> void
  def bar # error: Methods declared @abstract with an RBS comment must always raise
  end

  # @abstract
  #: -> void
  def baz # error: Methods declared @abstract with an RBS comment must always raise
    puts # error: Abstract methods must not contain any code in their body
  end

  # @abstract
  #: -> void
  def qux # error: Methods declared @abstract with an RBS comment must always raise
    puts # error: Abstract methods must not contain any code in their body
    puts
  end

  # @abstract
  #: -> void
  def self.foo; end # error: Methods declared @abstract with an RBS comment must always raise

  # @abstract
  #: -> void
  def self.bar # error: Methods declared @abstract with an RBS comment must always raise
  end

  # @abstract
  #: -> void
  def self.baz # error: Methods declared @abstract with an RBS comment must always raise
    puts # error: Abstract methods must not contain any code in their body
  end

  # @abstract
  #: -> void
  def self.qux # error: Methods declared @abstract with an RBS comment must always raise
    puts # error: Abstract methods must not contain any code in their body
    puts
  end
end

module ShimForAbstractModifierSupport
  # We can't use `T::DefMods` directly (since it's not in Sorbet's payload), so we provide a no-op shim just like it.
  def abstract(method_name) = method_name
end

# @abstract
class AbstractKeyword
  extend ShimForAbstractModifierSupport

  #: -> void
  abstract def foo; end

  #: -> void
  abstract def bar
  end

  #: -> void
  abstract def baz
    puts # error: Abstract methods must not contain any code in their body
  end

  #: -> void
  abstract def qux
    puts # error: Abstract methods must not contain any code in their body
    puts
  end

  #: -> void
  abstract def self.foo; end

  #: -> void
  abstract def self.bar
  end

  #: -> void
  abstract def self.baz
    puts # error: Abstract methods must not contain any code in their body
  end

  #: -> void
  abstract def self.qux
    puts # error: Abstract methods must not contain any code in their body
    puts
  end
end
