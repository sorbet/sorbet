# typed: strict
# selective-apply-code-action: refactor.extract

module Foo
  extend T::Sig

  sig {void}
  private_class_method def self.bar
                     # | apply-code-action: [A] Move method to a new module
  end
end
