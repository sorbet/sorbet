# typed: true

class A
  def self.m1(**nil)
  end

  def self.m2(*a, **nil)
  end

  def self.m3(a, **nil)
  end

  def self.m4(a, b, c, **nil)
  end

  def self.m5(a, b, *rest, **nil)
  end
end

def main
  hash1 = {}
  hash2 = {a: 10, b: 10}

  A.m1
  A.m1(*T.unsafe([]))
  A.m1(*[1, 2])
  A.m1(10) # error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(nil) # error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(10, a: 10) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `2`
  A.m1(a: nil) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(a: 10) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(a: 10, b: 10) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1({}) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1({a: 10}) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1({a: 10, b: 10}) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(**{}) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(**{a: 10}) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(**{a: 10, b: 10}) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(**hash1) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(**hash2) # error: No keyword accepted for method `A.m1`
# ^^^^^^^^^^^^^ error: Too many arguments provided for method `A.m1`. Expected: `0`, got: `1`
  A.m1(*[1, 2], **{}) # error: No keyword accepted for method `A.m1`
  A.m1(*[1, 2], **{a: 10, b: 10}) # error: No keyword accepted for method `A.m1`
  A.m1(*[1, 2], **hash1) # error: No keyword accepted for method `A.m1`
  A.m1(*[1, 2], **hash2) # error: No keyword accepted for method `A.m1`

  A.m2
  A.m2(*T.unsafe([]))
  A.m2(*[1, 2])
  A.m2(10)
  A.m2(nil)
  A.m2(a: nil) # error: No keyword accepted for method `A.m2`
  A.m2(a: 10) # error: No keyword accepted for method `A.m2`
  A.m2(a: 10, b: 10) # error: No keyword accepted for method `A.m2`
  A.m2({}) # error: No keyword accepted for method `A.m2`
  A.m2({a: 10}) # error: No keyword accepted for method `A.m2`
  A.m2({a: 10, b: 10}) # error: No keyword accepted for method `A.m2`
  A.m2(**{}) # error: No keyword accepted for method `A.m2`
  A.m2(**{a: 10}) # error: No keyword accepted for method `A.m2`
  A.m2(**{a: 10, b: 10}) # error: No keyword accepted for method `A.m2`
  A.m2(**hash1) # error: No keyword accepted for method `A.m2`
  A.m2(**hash2) # error: No keyword accepted for method `A.m2`
  A.m2(*[1, 2], **{}) # error: No keyword accepted for method `A.m2`
  A.m2(*[1, 2], **{a: 10, b: 10}) # error: No keyword accepted for method `A.m2`
  A.m2(*[1, 2], **hash1) # error: No keyword accepted for method `A.m2`
  A.m2(*[1, 2], **hash2) # error: No keyword accepted for method `A.m2`

  A.m3 # error: Not enough arguments provided for method `A.m3`. Expected: `1`, got: `0`
  A.m3(*T.unsafe([]))
  A.m3(*[1, 2])
  A.m3(10)
  A.m3(nil)
  A.m3(a: nil) # error: No keyword accepted for method `A.m3`
  A.m3(a: 10) # error: No keyword accepted for method `A.m3`
  A.m3(a: 10, b: 10) # error: No keyword accepted for method `A.m3`
  A.m3({}) # error: No keyword accepted for method `A.m3`
  A.m3({a: 10}) # error: No keyword accepted for method `A.m3`
  A.m3({a: 10, b: 10}) # error: No keyword accepted for method `A.m3`
  A.m3(**{}) # error: No keyword accepted for method `A.m3`
  A.m3(**{a: 10}) # error: No keyword accepted for method `A.m3`
  A.m3(**{a: 10, b: 10}) # error: No keyword accepted for method `A.m3`
  A.m3(**hash1) # error: No keyword accepted for method `A.m3`
  A.m3(**hash2) # error: No keyword accepted for method `A.m3`
  A.m3(*[1, 2], **{}) # error: No keyword accepted for method `A.m3`
  A.m3(*[1, 2], **{a: 10, b: 10}) # error: No keyword accepted for method `A.m3`
  A.m3(*[1, 2], **hash1) # error: No keyword accepted for method `A.m3`
  A.m3(*[1, 2], **hash2) # error: No keyword accepted for method `A.m3`

  A.m4 # error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `0`
  A.m4(*T.unsafe([]))
  A.m4(*[1, 2])
  A.m4(10) # error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(nil) # error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(10, 10, nil)
  A.m4(10, 10, nil, nil) # error: Too many arguments provided for method `A.m4`. Expected: `3`, got: `4`
  A.m4(a: nil) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(a: 10) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(a: 10, b: 10) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4({}) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4({a: 10}) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4({a: 10, b: 10}) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(**{}) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(**{a: 10}) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(**{a: 10, b: 10}) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(**hash1) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(**hash2) # error: No keyword accepted for method `A.m4`
# ^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m4`. Expected: `3`, got: `1`
  A.m4(*[1, 2], **{}) # error: No keyword accepted for method `A.m4`
  A.m4(*[1, 2], **{a: 10, b: 10}) # error: No keyword accepted for method `A.m4`
  A.m4(*[1, 2], **hash1) # error: No keyword accepted for method `A.m4`
  A.m4(*[1, 2], **hash2) # error: No keyword accepted for method `A.m4`

  A.m5 # error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `0`
  A.m5(*T.unsafe([]))
  A.m5(*[1, 2])
  A.m5(10) # error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(nil) # error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(10, 10, nil)
  A.m5(10, 10, nil, nil)
  A.m5(a: nil) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(a: 10) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(a: 10, b: 10) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5({}) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5({a: 10}) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5({a: 10, b: 10}) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(**{}) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(**{a: 10}) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(**{a: 10, b: 10}) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(**hash1) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(**hash2) # error: No keyword accepted for method `A.m5`
# ^^^^^^^^^^^^^ error: Not enough arguments provided for method `A.m5`. Expected: `2+`, got: `1`
  A.m5(*[1, 2], **{}) # error: No keyword accepted for method `A.m5`
  A.m5(*[1, 2], **{a: 10, b: 10}) # error: No keyword accepted for method `A.m5`
  A.m5(*[1, 2], **hash1) # error: No keyword accepted for method `A.m5`
  A.m5(*[1, 2], **hash2) # error: No keyword accepted for method `A.m5`
end
