# typed: strict
class Module; include T::Sig; end

class Parent
  sig {void}
  def supper
    # Previously, Sorbet would include this in the "did you mean" suggestions,
    # because `supper` is like `<super>`
  end
end

class Child < Parent
  sig {void}
  def no_parent_method
    super
  end
end
