# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class MyGenericClass
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig do
    type_parameters(:U)
      .params(elem: Elem, u: T.type_parameter(:U))
      .void
  end
  def caller(elem, u)
    create_me!(elem, u, "hello")
#   ^^^^^^^^^^ error: Method `create_me!` does not exist on `MyGenericClass[MyGenericClass::Elem]`
#     ^ apply-code-action: [A] Create missing method
  end
end
