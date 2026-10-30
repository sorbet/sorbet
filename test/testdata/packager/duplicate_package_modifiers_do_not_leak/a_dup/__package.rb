# typed: strict

# ...so this redefinition's `export_all!` must not apply to `A`.
class A < PackageSpec # error: Redefinition of package `A`
  export_all!
end
