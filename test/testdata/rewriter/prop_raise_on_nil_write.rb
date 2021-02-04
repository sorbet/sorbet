# typed: strict
class Thing
  include T::Props
  prop :foo, T.nilable(String), raise_on_nil_write: true
end