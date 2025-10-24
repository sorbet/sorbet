# typed: strict
# disable-fast-path: true

class Parent
  extend T::Generic
  XT = type_template
  XM = type_member
end

class Child < Parent
  XT = type_template(:out) # error: Type variance mismatch for `XT` with parent `T.class_of(Parent)`. Child `T.class_of(Child)` should be `invariant`, but it is `:out`
  XM = type_member(:out) # error: Type variance mismatch for `XM` with parent `Parent`. Child `Child` should be `invariant`, but it is `:out`
end
