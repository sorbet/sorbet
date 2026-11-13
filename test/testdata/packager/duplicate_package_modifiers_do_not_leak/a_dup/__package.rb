# typed: strict

# ...so this redefinition's `prelude!` must not apply to `A`.
class A < PackageSpec # error: Redefinition of package `A`
  prelude!
end
