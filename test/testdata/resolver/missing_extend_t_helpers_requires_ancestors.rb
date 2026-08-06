# typed: true
# enable-experimental-requires-ancestor: true

module RequiresAncestor
  requires_ancestor { Kernel }
# ^^^^^^^^^^^^^^^^^ error: Method `requires_ancestor` does not exist on `T.class_of(RequiresAncestor)`
end
