# typed: true
# selective-apply-code-action: quickfix
# keep-apply-all-quickfix: true

class Test::A::FooTest
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [E] Test Import `B` in package `A`
# ^^^^^^ apply-code-action: [F] Apply all Sorbet fixes for file
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [G] Test Import `B` in package `A`
# ^^^^^^ apply-code-action: [H] Apply all Sorbet fixes for file
end
