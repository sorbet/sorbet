# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
  export MyPackage::A::B
  export MyPackage::A::C
  export MyPackage::A
end
