# typed: strict

class Root::B < PackageSpec
  import Root, uses_internals: true
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Non-`test!` package `Root::B` cannot have imports with `uses_internals: true`
end
