# typed: true
# selective-apply-code-action: refactor.extract
#
# The test is asserting we can move a method without a sig

module Foo
  extend T::Sig

  def self.bar
         # | apply-code-action: [A] Move method to a new module
    'Hello'
  end
end

