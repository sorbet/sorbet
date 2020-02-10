# typed: true

class A
  extend T::Sig

  sig {params(x: T.attached_class).void}
            # ^ error: `T.attached_class` may only be used in an `:out` context
  def self.test(x); end
end
