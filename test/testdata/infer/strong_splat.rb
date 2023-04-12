# typed: strong
extend T::Sig

sig {void}
def example
  T.unsafe(Kernel).exec(["pay", "pay"], *ARGV)
# ^^^^^^^^^^^^^^^^ error: Call to method `exec` on `T.untyped`

  Kernel.exec(*T.unsafe([]))
# ^ error: Call to method `to_a` on `T.untyped`
# ^ error: Call to method `exec` with `T.untyped` splat arguments

  f = ->() {}
  T.unsafe(Kernel).exec(["pay", "pay"], &f)
# ^^^^^^^^^^^^^^^^ error: Call to method `exec` on `T.untyped`

  f = ->() {}
  T.unsafe(Kernel).exec(["pay", "pay"], *ARGV, &f)
# ^^^^^^^^^^^^^^^^ error: Call to method `exec` on `T.untyped`

  Kernel.exec(*T.unsafe([]), &f)
# ^ error: Call to method `to_a` on `T.untyped`
# ^ error: Call to method `exec` with `T.untyped` splat arguments
end
