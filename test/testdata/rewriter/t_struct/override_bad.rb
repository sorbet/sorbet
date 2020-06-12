# typed: strict

# TODO enable on the fast path

class OverrideBad < T::Struct
  extend T::Sig

  # This error about declaring instance variables inside `initialize` is because
  # technically the DSL pass for prop defines this instance variable inside the
  # mangled initialize$1 method.
  prop :foo, Integer # error: The instance variable `@foo` must be declared inside `initialize` or declared nilable

  sig {void}
  def initialize # error: Method `OverrideBad#initialize` redefined without matching argument count. Expected: `1`, got: `0`
  end
end
