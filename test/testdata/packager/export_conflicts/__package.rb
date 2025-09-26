# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
  export MyPackage::A::B
# ^^^^^^^^^^^^^^^^^^^^^^ error: Cannot export `MyPackage::A::B` because another exported name `MyPackage::A` is a prefix of it
  export MyPackage::A::C
# ^^^^^^^^^^^^^^^^^^^^^^ error: Cannot export `MyPackage::A::C` because another exported name `MyPackage::A` is a prefix of it
  export MyPackage::A

  export MyPackage::HasSingletonIvar # error: Cannot export `T.class_of(MyPackage::HasSingletonIvar::Nested)#@nested` because another exported name `MyPackage::HasSingletonIvar` is a prefix of it
end
