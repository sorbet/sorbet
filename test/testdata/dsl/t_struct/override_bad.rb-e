# typed: true
# disable-fast-path: true
# TODO enable on the fast path

class OverrideBad < T::Struct
  prop :foo, Integer

  def initialize # error: Method `OverrideBad#initialize` redefined without matching argument count. Expected: `1`, got: `0`
  end
end
