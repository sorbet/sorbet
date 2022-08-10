# typed: true
# spacer for exclude-from-file-update

class Foo
  extend T::Sig
  include IFoo

  sig {override.void}
  def foo; end
# ^^^^^^^ error: Method `Foo#foo` is marked `override` but does not override anything
end
