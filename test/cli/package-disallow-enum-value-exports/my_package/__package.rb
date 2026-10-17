# typed: strict

class MyPackage < PackageSpec
  export MyPackage::A::Val1 # error
  export MyPackage::A::NOT_A_VALUE
end
