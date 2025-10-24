# typed: strict
# disable-fast-path: true

class Parent
  extend T::Generic
  XT = type_template
  XM = type_member
end

class Child < Parent
  XT = type_template(:out)
  XM = type_member(:out) # error: Type variance mismatch for `XM` with parent `Parent`. Child `Child` should be `invariant`, but it is `:out`
end
