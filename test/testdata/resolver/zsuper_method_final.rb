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
  private :final_in_parent # error: `Parent#final_in_parent` was declared as final and cannot be marked `private` in `Child1`

  private_class_method :final_singleton_in_parent # error: `Parent.final_singleton_in_parent` was declared as final and cannot be marked `private` in `T.class_of(Child1)`

  public :private_final_in_parent # error: `Parent#private_final_in_parent` was declared as final and cannot be marked `public` in `Child1`
end

class Child2 < Parent
  class << self
    # This is the bug for the error with dupes: https://github.com/sorbet/sorbet/issues/3692
    private :final_singleton_in_parent # error-with-dupes: `Parent.final_singleton_in_parent` was declared as final and cannot be marked `private` in `T.class_of(Child2)`
  end
end
