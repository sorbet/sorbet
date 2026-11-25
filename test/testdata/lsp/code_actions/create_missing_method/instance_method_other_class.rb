# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class Target
  extend T::Sig

  sig { void }
  def existing_method
  end
end

class Caller
  extend T::Sig

  sig { params(target: Target).void }
  def caller(target)
    target.do_stuff(42, "hello")
#          ^^^^^^^^ error: Method `do_stuff` does not exist on `Target`
#            ^ apply-code-action: [A] Create missing method
  end
end
