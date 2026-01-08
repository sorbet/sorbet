# typed: true
# selective-apply-code-action: quickfix
# keep-apply-all-quickfix: true

class Test::A::FooTest
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [AA] Test Import `B` in package `A`
# ^^^^^^ apply-code-action: [BB] Apply all Sorbet fixes for file
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [CC] Test Import `B` in package `A`
# ^^^^^^ apply-code-action: [DD] Apply all Sorbet fixes for file
  C::Foo.new
# ^^^^^^ error: `C::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [EE] Test Import `C` in package `A`
# ^^^^^^ apply-code-action: [FF] Apply all Sorbet fixes for file
end
