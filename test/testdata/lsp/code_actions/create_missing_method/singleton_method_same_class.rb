# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class MyClass
  extend T::Sig

  sig { void }
  def caller
    MyClass.do_class_thing("hello", 42)
#           ^^^^^^^^^^^^^^ error: Method `do_class_thing` does not exist on `T.class_of(MyClass)`
#             ^ apply-code-action: [A] Create missing method
  end

  sig { void }
  def other_method
  end
end
