# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
  strict_exports true
  export MyPackage::A::B
  export MyPackage::A::C
  export MyPackage::A
end
