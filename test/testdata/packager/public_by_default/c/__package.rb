# typed: strict

class Root::C < PackageSpec
  # `export_all!` mixed with explicit exports.
  export_all!
# ^^^^^^^^^^^ error: Package `Root::C` declares `export_all!` and therefore should not use explicit exports
  export Root::C::Unexported
end
