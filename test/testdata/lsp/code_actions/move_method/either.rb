# typed: strict
# selective-apply-code-action: refactor.extract

module Either
  extend T::Sig

  class Left
    extend T::Sig
    sig {void}
    def self.foo; end
      # | apply-code-action: [A] Move method to a new module
  end

  class Right
    extend T::Sig
    sig {void}
    def self.foo; end
  end

  sig {params(x: T.any(T.class_of(Left), T.class_of(Right))).void}
  def example(x)
    x.foo
  end
end
