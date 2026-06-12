# typed: strict

class Opus::Util  < PackageSpec
  export Opus::Util::UtilClass

  # Allowed export for test a prefix of a public export
  export Opus::Util::Nesting::Public
end
