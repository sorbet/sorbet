# typed: true
# selective-apply-code-action: quickfix
# keep-apply-all-quickfix: true

class A::Foo
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [A] Import `B` in package `A`
# ^^^^^^ apply-code-action: [B] Apply all Sorbet fixes for file
  B::Foo.new
# ^^^^^^ error: `B::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [C] Import `B` in package `A`
# ^^^^^^ apply-code-action: [D] Apply all Sorbet fixes for file
  C::Foo.new
# ^^^^^^ error: `C::Foo` resolves but its package is not imported
# ^^^^^^ apply-code-action: [E] Import `C` in package `A`
# ^^^^^^ apply-code-action: [F] Apply all Sorbet fixes for file
  D::Foo.new
# ^^^^^^ error: `D::Foo` resolves but is not exported from `D`
# ^^^^^^ apply-code-action: [G] Export `D::Foo` in package `D`
# ^^^^^^ apply-code-action: [H] Apply all Sorbet fixes for file
  D::Foo.new
# ^^^^^^ error: `D::Foo` resolves but is not exported from `D`
# ^^^^^^ apply-code-action: [I] Export `D::Foo` in package `D`
# ^^^^^^ apply-code-action: [J] Apply all Sorbet fixes for file
end
