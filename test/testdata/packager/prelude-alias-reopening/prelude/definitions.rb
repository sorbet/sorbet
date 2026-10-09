# typed: true

# The payload's Queue alias already resolves to Thread::Queue during naming.
class ::Queue::AliasControl
end
::Queue::ALIAS_CONTROL = T.let(1, Integer)

# A synthetic reference through the alias is not itself a definition.
class Queue::Rejected # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end
Queue::REJECTED = T.let(2, Integer) # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
