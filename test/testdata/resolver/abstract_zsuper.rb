# typed: false
require 'sorbet-runtime'

class Parent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def abstract_in_parent; end
end

class Child < Parent # error: Missing definition for abstract method `abstract_in_parent`
  private :abstract_in_parent
end
