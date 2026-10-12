# typed: true

# The payload's Queue alias already resolves to Thread::Queue during naming.
class ::Queue::AliasControl
#     ^^^^^^^ error: Redefining constant `Queue` as a class or module
end
::Queue::ALIAS_CONTROL = T.let(1, Integer) # error: Redefining constant `Queue` as a class or module

# Unqualified definitions through an alias also check their package namespace.
class Queue::Rejected # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
#     ^^^^^ error: Redefining constant `Queue` as a class or module
end
  Queue::REJECTED = T.let(2, Integer)
# ^^^^^ error: Redefining constant `Queue` as a class or module
# ^^^^^^^^^^^^^^^ error: File belongs to package `Prelude` but defines a constant that does not match this namespace
