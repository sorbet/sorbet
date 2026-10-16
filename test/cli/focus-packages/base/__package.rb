# typed: strict

class Lib::Base < PackageSpec
  import RootDep
  export Lib::Base::Value
end
