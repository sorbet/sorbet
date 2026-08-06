# typed: true

class C
  extend T::Helpers
  def self.void; end # define this so `void` below doesn't autocorrect to `load`

  sig { void }
# ^^^ error: Method `sig` does not exist on `T.class_of(C)`
  def foo; end
end
