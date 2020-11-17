# typed: false
require 'sorbet-runtime'

T::Configuration.enable_final_checks_on_hooks

class Parent
  extend T::Sig

  sig(:final) {void}
  def final_in_parent; end

  sig(:final) {void}
  def self.final_singleton_in_parent; end

  sig(:final) {void}
  private def private_final_in_parent; end
end

class Child1 < Parent
  # We have tests that this is an error at runtime, too.
  private :final_in_parent
  #        ^ error: `Parent#final_in_parent` was declared as final and cannot be marked private in `Child`

  private_class_method :final_singleton_in_parent
  #                     ^ error: `Parent#final_singleton_in_parent` was declared as final and cannot be marked private in `Child`

  public :private_final_in_parent
  #       ^ error: `Parent#private_final_in_parent` was declared as final and cannot be marked public in `Child`
end

class Child2 < Parent
  class << self
    private :final_singleton_in_parent
    #        ^ error: `Parent.final_singleton_in_parent` was declared as final and cannot be marked `private` in `T.class_of(Child2)`
  end
end
