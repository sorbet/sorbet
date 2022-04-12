# frozen_string_literal: true

# typed: true
# selective-apply-code-action: quickfix

class Foo
  def bar(n); end
end

def test
  Foo.new.baz(42)
#         ^^^     error: Method `baz` does not exist on `Foo`
#         ^^^     apply-code-action: [A] Replace with `bar`

  Foo.net
#     ^^^ error: Method `net` does not exist on `T.class_of(Foo)`
#     ^^^ apply-code-action: [B] Replace with `new`

  Boo.new
# ^^^ error: Unable to resolve constant `Boo`
# ^^^ apply-code-action: [C] Replace with `Foo`
end
