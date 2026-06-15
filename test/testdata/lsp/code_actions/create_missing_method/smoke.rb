# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-extract-to-variable: true
# TODO(bshu): incorrect behavior

module Testing
  extend T::Sig
  extend T::Generic

  sig do
    type_parameters(:T)
      .params(x: String, y: Integer, t: T.type_parameter(:T))
      .void
  end
  def hello(x, y, t)
      create_me!(true, "1234234", 1, x, x, y, t, hello: 1243234)
#     ^^^^^^^^^^ error: Method `create_me!` does not exist on `Testing`
#       ^ apply-code-action: [A] Create missing method
  end
end
