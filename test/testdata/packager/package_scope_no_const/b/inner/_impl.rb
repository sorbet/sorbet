# typed: false

class B
  Inner = type_member
# ^^^^^ error: File belongs to package `B::Inner` but defines a constant that does not match this namespace
#         ^^^^^^^^^^^ error: `type_member` may only be used on constants in the package that owns them

end
