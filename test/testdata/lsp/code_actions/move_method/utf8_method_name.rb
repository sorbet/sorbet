# typed: strict
# selective-apply-code-action: refactor.extract

module Foo
  extend T::Sig
  sig {void}
  def self.ж; end
     # ^ apply-code-action: [A] Move method to a new module

  sig {void}
  def self.😱; end
     # ^ apply-code-action: [B] Move method to a new module
end


Foo.ж
Foo.😱

