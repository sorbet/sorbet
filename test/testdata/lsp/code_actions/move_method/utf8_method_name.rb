# typed: strict
# selective-apply-code-action: refactor.extract
#
# Test asserts that non-alphanumeric characters doesn't appear in the new module name

module Foo
  extend T::Sig
  sig {void}
  def self.ж; end
     # | apply-code-action: [A] Move method to a new module

  sig {void}
  def self.😱; end
     # | apply-code-action: [B] Move method to a new module

  sig {void}
  def self.method_with_?; end
     # | apply-code-action: [C] Move method to a new module

end


Foo.ж
Foo.😱
Foo.method_with_?
