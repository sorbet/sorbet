# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class MixedArgs
  extend T::Sig

  sig { void }
  def caller
    send_request("POST", "/api", body: "data", timeout: 30)
#   ^^^^^^^^^^^^ error: Method `send_request` does not exist on `MixedArgs`
#     ^ apply-code-action: [A] Create missing method
  end
end
