# typed: true
extend T::Sig

# An invalid `has_attached_class!` must not leave behind an `<AttachedClass>`
# member that isn't a type member.

  has_attached_class!
# ^^^^^^^^^^^^^^^^^^^ error: `type_member` cannot be used at the top-level
# ^^^^^^^^^^^^^^^^^^^ error: Method `has_attached_class!` does not exist on `T.class_of(<root>)`

  sig { returns(T.attached_class) }
  #             ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in singleton methods on classes or instance methods on `has_attached_class!` modules
  def at_top_level = T.unsafe(nil)

module TooManyArgs
  extend T::Sig
  extend T::Generic

  has_attached_class!(:out, 1)
  #                         ^ error: Too many arguments in `type_member` definition for `<AttachedClass>`
  #                         ^ error: Too many arguments provided for method `T::Generic#has_attached_class!`. Expected: `0..1`, got: `2`

  sig { returns(T.attached_class) }
  #             ^^^^^^^^^^^^^^^^ error: `TooManyArgs` must declare `has_attached_class!` before module instance methods can use `T.attached_class`
  def in_module = T.unsafe(nil)
end
