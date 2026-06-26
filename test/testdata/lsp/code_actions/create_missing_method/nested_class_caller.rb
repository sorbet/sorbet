# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

module Outer
  class Inner
    extend T::Sig

    sig { void }
    def outer_method
      inner_method("hello")
#     ^^^^^^^^^^^^ error: Method `inner_method` does not exist on `Outer::Inner`
#       ^ apply-code-action: [A] Create missing method
    end

    sig { void }
    def other_method
    end
  end
end
