# typed: true

module Interface
  extend T::Sig
  extend T::Helpers

  interface!

  sig.abstract { void }
  def foo; end
end

class Override
  extend T::Sig

  include Interface

  sig.override { void }
  def foo; end

  sig.overridable { void }
  def bar; end
end

class Incompatible
  extend T::Sig

  include Interface

  sig.override(allow_incompatible: true) { returns(Integer) }
  def foo
    123
  end
end

class Final
  extend T::Sig

  sig.final { void }
  def foo; end
end