# typed: true

class AbstractClass
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(Object)}
  def self.foo; end

  sig {abstract.returns(Object)}
  def bar; end
end

class ImplEnabling < AbstractClass
  sig {override(allow_incompatible: false).params(x: Integer).returns(Object)}
  #                                 ^^^^^ error: `override(allow_incompatible: ...)` expects either `true` or `:visibility`
  def self.foo(x); end # error: `AbstractClass.foo` must accept no more than `0` required argument(s)

  sig {override(allow_incompatible: true).params(x: Integer).returns(Object)}
  def bar(x); end
end
