# typed: true

# Reject the source namespace regardless of whether its aliases have resolved.
class A::Same::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
#     ^^^^^^^ error: Redefining constant `Same` as a class or module
end
  A::Same::FIELD = 1
# ^^^^^^^ error: Redefining constant `Same` as a class or module
# ^^^^^^^^^^^^^^ error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
class A::Foreign::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
#     ^^^^^^^^^^ error: Redefining constant `Foreign` as a class or module
end
  A::Foreign::FIELD = 1
# ^^^^^^^^^^ error: Redefining constant `Foreign` as a class or module
# ^^^^^^^^^^^^^^^^^ error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
class A::Chained::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
#     ^^^^^^^^^^ error: Redefining constant `Chained` as a class or module
end
  A::Chained::FIELD = 1
# ^^^^^^^^^^ error: Redefining constant `Chained` as a class or module
# ^^^^^^^^^^^^^^^^^ error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
module A::ModuleAlias # error: Redefining constant `ModuleAlias` as a class or module
# error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
  def alias_recovery_marker; end
end

# An unpackaged target must not make the foreign source namespace permissible.
class A::Unpackaged::Nested # error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
#     ^^^^^^^^^^^^^ error: Redefining constant `Unpackaged` as a class or module
end
  A::Unpackaged::FIELD = 1
# ^^^^^^^^^^^^^ error: Redefining constant `Unpackaged` as a class or module
# ^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `A::Patch` but defines a constant that does not match this namespace
