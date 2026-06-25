# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class MyClass
  extend T::Sig

  do_something("hello")
# ^^^^^^^^^^^^ error: Method `do_something` does not exist on `T.class_of(MyClass)`
#   ^ apply-code-action: [A] Create missing method

  sig { void }
  def other_method
  end
end
