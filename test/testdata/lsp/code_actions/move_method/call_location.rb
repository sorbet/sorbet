# typed: strict
# selective-apply-code-action: refactor.extract

module Foo
  extend T::Sig

  sig {void}
  def self.a
# | apply-code-action: [A] Move method to a new module
  end

  sig {void}
  def self.b
    # | apply-code-action: [B] Move method to a new module
  end

  sig {void}
  def self.c
        # | apply-code-action: [C] Move method to a new module
  end

  sig {void}
  def self.d
         # | apply-code-action: [D] Move method to a new module
  end
end
