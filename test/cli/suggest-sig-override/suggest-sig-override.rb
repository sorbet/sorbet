# typed: strict

extend T::Helpers

# Suggest "implementation." for these classes
module Abstract
  extend T::Helpers
  abstract!
  sig {abstract.void}
  def foo; end
end
class Implementation
  include Abstract

  def foo; end
end

# Suggest "override." for these classes
class Parent
  extend T::Helpers
  sig {void}
  def initialize; end

  sig {overridable.void}
  def foo; end

  sig {void}
  def bar; end

  def qux; end
end
class Child < Parent
  def initialize; end

  def foo; end

  def bar; end

  def qux; end
end
