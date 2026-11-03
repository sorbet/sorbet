# typed: true

# These calls fail because the receiver is not a singleton class, so there's
# no class to suggest inserting `extend T::Helpers` or `extend T::Sig` into.

module M
  def g
    mixes_in_class_methods(Comparable)
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `mixes_in_class_methods` can only be declared inside a module, not a class
  # ^^^^^^^^^^^^^^^^^^^^^^ error: Method `mixes_in_class_methods` does not exist on `M`
  end
end

1.abstract!
# ^^^^^^^^^ error: Method `abstract!` does not exist on `Integer` component of `Integer(1)`

1.sig {}
# ^^^ error: Method `sig` does not exist on `Integer` component of `Integer(1)`
