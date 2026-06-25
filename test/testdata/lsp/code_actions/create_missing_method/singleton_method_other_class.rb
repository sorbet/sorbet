# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class Other
  extend T::Sig

  sig { void }
  def existing_method
  end
end

class Caller
  extend T::Sig

  sig { void }
  def caller
    Other.do_class_thing("hello", 42)
#         ^^^^^^^^^^^^^^ error: Method `do_class_thing` does not exist on `T.class_of(Other)`
#           ^ apply-code-action: [A] Create missing method
  end
end
