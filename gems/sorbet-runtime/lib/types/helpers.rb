# frozen_string_literal: true
# typed: true

# Use as a mixin with extend (`extend T::Helpers`).
# Docs at https://confluence.corp.stripe.com/display/PRODINFRA/Ruby+Types
module T::Helpers
  Private = T::Private

  ### Class/Module Helpers ###

  def abstract!
    Private::Abstract::Declare.declare_abstract(self, type: :abstract)
  end

  def interface!
    Private::Abstract::Declare.declare_abstract(self, type: :interface)
  end

  # Causes a mixin to also mix in class methods from the named module.
  #
  # Nearly equivalent to
  #
  #  def self.included(other)
  #    other.extend(mod)
  #  end
  #
  # Except that it is statically analyzed by sorbet.
  def mixes_in_class_methods(mod)
    Private::Mixins.declare_mixes_in_class_methods(self, mod)
  end
end
