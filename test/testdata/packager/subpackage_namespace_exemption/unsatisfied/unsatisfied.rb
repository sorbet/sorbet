# typed: true

module Unsatisfied
  extend T::Generic # error: `T.class_of(Unsatisfied)` can only have ancestors if every package nested under `Unsatisfied` imports it
  Elem = type_member # error: `Unsatisfied` can only be generic if every package nested under `Unsatisfied` imports it
end
