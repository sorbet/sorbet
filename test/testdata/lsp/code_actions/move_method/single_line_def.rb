# typed: strict
# selective-apply-code-action: refactor.extract
#
# The test is asserting we can't move a method without a sig

module Bar
  extend T::Sig

  sig {void}
  def self.bar; end
         # ^^^ apply-code-action: [A] Move method to a new module
end
