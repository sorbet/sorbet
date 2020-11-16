# typed: false
require 'sorbet-runtime'

T::Configuration.enable_final_checks_on_hooks

class Parent
  extend T::Sig

  sig(:final) {void}
  def final_in_parent; end
end

class Child < Parent
  # We have tests that this is an error at runtime, too.
  private :final_in_parent # error: `Parent#final_in_parent` was declared as final and cannot be overridden by `Child#final_in_parent`
end
