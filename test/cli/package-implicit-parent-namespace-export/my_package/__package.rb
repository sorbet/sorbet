# typed: strict

class MyPackage < PackageSpec
  export MyPackage::ParentNamespace::Const1
  export MyPackage::ParentNamespace::Const2
  export MyPackage::BehaviorDefiningParentNamespace::Const1
end

