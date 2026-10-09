# typed: true

# The payload's Queue alias already resolves to Thread::Queue during naming.
class ::Queue::AliasControl # error: Redefining constant `Queue` as a class or module
end
::Queue::ALIAS_CONTROL = T.let(1, Integer) # error: Redefining constant `Queue` as a class or module

# Definition scopes cannot follow aliases, even in prelude packages.
class Queue::Rejected # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end
Queue::REJECTED = T.let(2, Integer) # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
