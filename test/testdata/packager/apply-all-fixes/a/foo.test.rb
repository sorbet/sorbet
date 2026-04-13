# typed: true
# selective-apply-code-action: quickfix
# omit-apply-all-quickfix: false

class Test::A::FooTest
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [G] Test Import `B` in package `A`
# ^^^^^^ apply-code-action: [H] Apply all Sorbet fixes for file
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [I] Test Import `B` in package `A`
# ^^^^^^ apply-code-action: [J] Apply all Sorbet fixes for file
  C::Foo.new
# ^^^^^^ error: `C::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [K] Test Import `C` in package `A`
# ^^^^^^ apply-code-action: [L] Apply all Sorbet fixes for file
end
