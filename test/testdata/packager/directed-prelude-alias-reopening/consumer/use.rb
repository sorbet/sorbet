# typed: true

# Explicit-root definitions through aliases are also isolated.
T.let(Queue::AliasControl.new, Thread::Queue::AliasControl) # error: Unable to resolve constant `AliasControl`
#                              ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `AliasControl`
T.let(Queue::ALIAS_CONTROL, Integer) # error: Unable to resolve constant `ALIAS_CONTROL`
T.let(Thread::Queue::ALIAS_CONTROL, Integer) # error: Unable to resolve constant `ALIAS_CONTROL`

# The non-root definitions remain isolated.
Thread::Queue::Rejected # error: Unable to resolve constant `Rejected`
Thread::Queue::REJECTED # error: Unable to resolve constant `REJECTED`
Thread::Queue.new.alias_recovery_marker # error: Method `alias_recovery_marker` does not exist
