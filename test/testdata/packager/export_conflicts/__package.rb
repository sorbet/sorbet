# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
  export MyPackage::A::B
       # ^^^^^^^^^^^^^^^ error: Exported names may not be prefixes of each other
  export MyPackage::A::C
       # ^^^^^^^^^^^^^^^ error: Exported names may not be prefixes of each other
  export MyPackage::A
end
