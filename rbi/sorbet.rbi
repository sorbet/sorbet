# This file contains constants and methods that are available for users to use
# in RBI files, so aren't present at runtime.

class Sorbet
  # Identical to `T::Helpers`'s `sig` in semantics, but couldn't work at
  # runtime since it doesn't know `self`. Used in `rbi`s that don't `extend
  # T::Helpers`.
  def self.sig(*args)
  end
end

class Sorbet::Private::Builder
  include T::Helpers
  sig(type: T.untyped).returns(Sorbet::Private::Builder)
  def returns(type); end;

  sig.returns(Sorbet::Private::Builder)
  def void; end;

  sig(arg: T.untyped).returns(Sorbet::Private::Builder)
  def checked(arg); end;

  sig(params: T.untyped).returns(Sorbet::Private::Builder)
  def soft(**params); end

  sig.returns(Sorbet::Private::Builder)
  def abstract; end;

  sig.returns(Sorbet::Private::Builder)
  def override; end;

  sig.returns(Sorbet::Private::Builder)
  def implementation; end;

  sig.returns(Sorbet::Private::Builder)
  def overridable; end;
end
