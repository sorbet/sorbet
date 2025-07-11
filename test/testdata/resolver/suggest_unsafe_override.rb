# typed: true
# enable-suggest-unsafe: true

class Parent
  extend T::Sig
  extend T::Helpers

  sig {params(x: String).void}
  def foo(x); end
end

# basic, no-args override
class Child1 < Parent
  extend T::Sig
  extend T::Helpers

  sig {override.params(x: T::Boolean).void}
#                      ^ error: Parameter `x` of type `T::Boolean` not compatible with type of overridden method `Parent#foo`
  def foo(x); end
end

# override with empty args
class Child2 < Parent
  extend T::Sig
  extend T::Helpers

  sig {override().params(x: T::Boolean).void}
#                        ^ error: Parameter `x` of type `T::Boolean` not compatible with type of overridden method `Parent#foo`
  def foo(x); end
end

# override with `allow_incompatible: false`
class Child3 < Parent
  extend T::Sig
  extend T::Helpers

  # The error-with-dupes below is because `--suggest-unsafe` runs `sig_finder`,
  # which calls type_syntax parsing, which unconditionally reports errors.
  # We probably want some way to swallow errors when sig_finder runs type_syntax parsing.
  sig {override(allow_incompatible: false).params(x: T::Boolean).void}
  #                                 ^^^^^ error-with-dupes: `override(allow_incompatible: ...)` expects either `true` or `:visibility`
  #                                               ^ error: Parameter `x` of type `T::Boolean` not compatible with type of overridden method `Parent#foo`
  def foo(x); end
end
