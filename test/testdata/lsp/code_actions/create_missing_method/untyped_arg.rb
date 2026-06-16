# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class MyClass
  extend T::Sig

  sig { params(x: T.untyped).void }
  def caller(x)
    y = T.cast(11234, T.untyped)
    do_stuff(x, y, "hello")
#   ^^^^^^^^ error: Method `do_stuff` does not exist on `MyClass`
#     ^ apply-code-action: [A] Create missing method
  end
end
