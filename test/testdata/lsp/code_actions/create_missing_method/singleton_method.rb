# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true
# TODO(bshu): incorrect behavior

class Other
  extend T::Sig
end

class Caller
  extend T::Sig

  sig { void }
  def call_it
    Other.do_thing("hello", 42)
#         ^^^^^^^^ error: Method `do_thing` does not exist on `T.class_of(Other)`
#           ^ apply-code-action: [A] Create missing method
  end
end
