# typed: false

class B
  Inner = type_member
# ^^^^^ error: File belongs to package `B::Inner` but defines a constant that does not match this namespace
# ^^^^^ error: `B::Inner` shares a name with a package and thus must be a class or module, not a constant assignment
end
