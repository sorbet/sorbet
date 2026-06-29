# typed: true
# selective-apply-code-action: refactor,refactor.rewrite
# enable-experimental-lsp-create-missing-method: true

class Foo
  def bar(x = baz())
#             ^^^ error: Method `baz` does not exist on `Foo`
#               | apply-code-action: [A] Create missing method
  end
end
