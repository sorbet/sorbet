# typed: true

# Valid explicit-root definitions reach the canonical target.
T.let(Queue::AliasControl.new, Thread::Queue::AliasControl)
T.let(Queue::ALIAS_CONTROL, Integer)
T.let(Thread::Queue::ALIAS_CONTROL, Integer)

# The non-root definitions remain isolated.
Thread::Queue::Rejected # error: Unable to resolve constant `Rejected`
Thread::Queue::REJECTED # error: Unable to resolve constant `REJECTED`
Thread::Queue.new.alias_recovery_marker # error: Method `alias_recovery_marker` does not exist
