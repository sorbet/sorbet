# typed: strict

class Root::B < PackageSpec
  import Root, uses_internals: true
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Root::B` may not define imports with `uses_internals: true`
end
