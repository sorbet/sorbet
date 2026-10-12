# typed: true

module A
  class Thing; end
  Same = Thing
# ^^^^^^^^^^^^ error: Cannot initialize the class or module `Same` by constant assignment
  Foreign = B::Thing
# ^^^^^^^^^^^^^^^^^^ error: Cannot initialize the class or module `Foreign` by constant assignment
  Chained = Foreign
# ^^^^^^^^^^^^^^^^^ error: Cannot initialize the class or module `Chained` by constant assignment
  ModuleAlias = B::Mixin
# ^^^^^^^^^^^^^^^^^^^^^^ error: Cannot initialize the module `ModuleAlias` by constant assignment
  Unpackaged = String
# ^^^^^^^^^^^^^^^^^^^ error: Cannot initialize the class or module `Unpackaged` by constant assignment
end
