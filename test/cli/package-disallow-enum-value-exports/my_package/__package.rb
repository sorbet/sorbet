# typed: strict

class MyPackage < PackageSpec
  export MyPackage::A::Val1 # error
end
