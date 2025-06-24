# typed: true
# enable-suggest-unsafe: true

class Parent
  extend T::Sig

  sig {overridable.returns(Integer)}
  def some_public_api; 0; end
end

class ChildBad < Parent
  sig {override.returns(Integer)}
  private def some_public_api; 1; end # error: Method `some_public_api` is private in `ChildBad` but not in `Parent`
end

class ChildOkay < Parent
  sig {override(allow_incompatible: :visibility).returns(Integer)}
  private def some_public_api; 1; end
end

# We want this to overwrite with `allow_incompatible: true` anyway.
class ChildBadTypes < Parent
  sig {override(allow_incompatible: :visibility).returns(String)}
  private def some_public_api; ''; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Return type `String` does not match return type of overridable method `Parent#some_public_api`
end

# This one will overwrite to `allow_incompatible: :visibility`, but w/e
class ChildBadSymbol < Parent
  sig {override(allow_incompatible: :bad).returns(Integer)}
  #                                 ^^^^ error-with-dupes: `override(allow_incompatible: ...)` expects either `true` or `:visibility
  private def some_public_api; 0; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Method `some_public_api` is private in `ChildBadSymbol` but not in `Parent`
end

class ChildBoth1 < Parent
  sig { override(allow_incompatible: true).override(allow_incompatible: :visibility).returns(Integer) }
  #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Don't use both `override(allow_incompatible: true)` and `override(allow_incompatible: :visibility)
  private def some_public_api; 0; end
end

class ChildBoth2 < Parent
  sig { override(allow_incompatible: :visibility).override(allow_incompatible: true).returns(Integer) }
  #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Don't use both `override(allow_incompatible: true)` and `override(allow_incompatible: :visibility)
  private def some_public_api; 0; end
end

class ChildBoth3 < Parent
  sig { returns(Integer).override(allow_incompatible: :visibility).override(allow_incompatible: true) }
  #                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Don't use both `override(allow_incompatible: true)` and `override(allow_incompatible: :visibility)
  private def some_public_api; 0; end
end

# This should be rewritten to `allow_incompatible: true`
class ChildBothErrors < Parent
  sig { override.returns(String) }
  private def some_public_api; ''; end
#         ^^^^^^^^^^^^^^^^^^^ error: Return type `String` does not match return type of overridable method `Parent#some_public_api`
#         ^^^^^^^^^^^^^^^^^^^ error: Method `some_public_api` is private in `ChildBothErrors` but not in `Parent`
end

# This previously crashed due to attempting to replace a non-existent `override`
class AbstractChild < Parent
  extend T::Helpers
  abstract!
  sig { abstract.returns(Integer) }
  private def some_public_api; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Method `some_public_api` is private in `AbstractChild` but not in `Parent`
  #       ^^^^^^^^^^^^^^^^^^^ error: Method `AbstractChild#some_public_api` overrides an overridable method `Parent#some_public_api` but is not declared with `override.`
end

# This previously crashed due to attempting to replace a non-existent `override`
class NonExplicitOverrideChild < Parent
  sig { returns(Integer) }
  private def some_public_api; 0; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Method `some_public_api` is private in `NonExplicitOverrideChild` but not in `Parent`
  #       ^^^^^^^^^^^^^^^^^^^ error: Method `NonExplicitOverrideChild#some_public_api` overrides an overridable method `Parent#some_public_api` but is not declared with `override.`
end
