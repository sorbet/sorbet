# typed: false

 A = 1
#^ error: File belongs to package `A` but defines a constant that does not match this namespace
#^ error: `A` shares a name with a package and thus must be a class or module, not a constant assignment
