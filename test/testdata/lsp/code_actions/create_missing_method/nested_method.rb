# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class MyClass
  extend T::Sig

  sig { void }
  def outer_method
    def inner_method
      do_something("hello", 42)
#     ^^^^^^^^^^^^ error: Method `do_something` does not exist on `MyClass`
#       ^ apply-code-action: [A] Create missing method
    end
    
    puts "asdf"
    puts "asdf"
    puts "asdf"
  end
end
