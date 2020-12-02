# typed: true

class Foo
  def call
#     ^ apply-rename: [A] newName: foo invalid: true expectedErrorMessage: The `call` method cannot be renamed.
  end

  def [](a)
#     ^ apply-rename: [B] newName: foo invalid: true expectedErrorMessage: The `[]` method cannot be renamed.
  end

  def +(a)
#     ^ apply-rename: [C] newName: foo invalid: true expectedErrorMessage: The `+` method cannot be renamed.
  end
end

class Bar
  # not called
  def call
#     ^ apply-rename: [D] newName: foo invalid: true expectedErrorMessage: The `call` method cannot be renamed.
  end
end

f = Foo.new
f.()
f[1]
f + f
