# typed: strict

class Root::C < PackageSpec
  # `export_all!` mixed with explicit exports.
  export_all!
  export Root::C::Unexported
end
