# typed: true
# (enable-experimental-requires-ancestor defaulted to false)

module Interface
  interface!
# ^^^^^^^^^^ error: Method `interface!` does not exist on `T.class_of(Interface)`
end

module AbstractModule
  abstract!
# ^^^^^^^^^ error: Method `abstract!` does not exist on `T.class_of(AbstractModule)`
end

class AbstractClass
  abstract!
# ^^^^^^^^^ error: Method `abstract!` does not exist on `T.class_of(AbstractClass)`
end

class FinalClass
  final!
# ^^^^^^ error: Method `final!` does not exist on `T.class_of(FinalClass)`
end

class SealedClass
  sealed!
# ^^^^^^^ error: Method `sealed!` does not exist on `T.class_of(SealedClass)`
end

module RequiresAncestor
  # No auto-correction, because enable-experimental-requires-ancestor is off by default.
  requires_ancestor { Kernel }
# ^^^^^^^^^^^^^^^^^ error: Method `requires_ancestor` does not exist on `T.class_of(RequiresAncestor)`
end

module MixesInClassMethods
  mixes_in_class_methods { Kernel }
# ^^^^^^^^^^^^^^^^^^^^^^ error: Method `mixes_in_class_methods` does not exist on `T.class_of(MixesInClassMethods)`
end
