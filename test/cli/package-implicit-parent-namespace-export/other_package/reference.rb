# typed: strict

module OtherPackage
  MPP = MyPackage::ParentNamespace # Allowed due to implicit parent namespace exporting

  MPP::Const1
  MPP::Const2
  MPP::PrivateConst3 # Should be an ERROR! Private constant.

  BPP = MyPackage::BehaviorDefiningParentNamespace # should be an ERROR! Not allowed since namespace defines behavior.

  BPP::Const1 # This is ok since MyPackage::BehaviorDefiningParentNamespace::Const1 is explicitly exported :)
end
