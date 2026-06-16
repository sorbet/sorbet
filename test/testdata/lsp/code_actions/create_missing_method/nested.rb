# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

module Outer
  class Inner
    extend T::Sig

    sig { void }
    def caller
      nested_call("hello")
#     ^^^^^^^^^^^ error: Method `nested_call` does not exist on `Outer::Inner`
#       ^ apply-code-action: [A] Create missing method
    end
  end
end
