# typed: true

class Parent
  extend T::Sig

  sig {overridable.returns(Integer)}
  def some_public_api; 0; end
end

class ChildBad < Parent
  sig {override.returns(Integer)}
  private def some_public_api; 1; end
end

class ChildOkay < Parent
  sig {override(allow_incompatible: :visibility).returns(Integer)}
  private def some_public_api; 1; end
end

class ChildBadTypes < Parent
  sig {override(allow_incompatible: :visibility).returns(String)}
  private def some_public_api; ''; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Return type `String` does not match return type of overridable method `Parent#some_public_api`
end

class ChildBadSymbol < Parent
  sig {override(allow_incompatible: :bad).returns(Integer)}
  #                                 ^^^^ error: `override(allow_incompatible: ...)` expects one of `true`, `false`, or `:visibility
  private def some_public_api; 0; end
end
