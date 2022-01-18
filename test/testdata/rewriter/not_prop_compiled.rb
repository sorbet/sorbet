# compiled: true
# typed: true

class ThingsWhichUsedToBePropSyntax
  prop :type, type: String # error: `prop` does not exist
  prop :object # error: `prop` does not exist
  prop :array_of, array: String # error: `prop` does not exist
  prop :array_of_explicit, Array, array: String
# ^^^^ error: `prop` does not exist
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `decorator` does not exist
  prop :no_class_arg, type: Array, immutable: true, array: String # error: `prop` does not exist
  prop :proc_type, type: T.proc.params(x: Integer).void # error: `prop` does not exist
  prop :enum_prop, enum: ["hello", "goodbye"] # error: `prop` does not exist
end
