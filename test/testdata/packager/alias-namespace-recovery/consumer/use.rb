# typed: true

# Reading through aliases remains valid.
T.let(A::Same.new, A::Thing)
T.let(A::Foreign.new, B::Thing)
T.let(A::Chained.new, B::Thing)
T.let(A::Unpackaged.new, String)

# Rejected definitions must leave the alias targets unchanged.
A::Thing::Nested # error: Unable to resolve constant `Nested`
A::Thing::FIELD # error: Unable to resolve constant `FIELD`
B::Thing::Nested # error: Unable to resolve constant `Nested`
B::Thing::FIELD # error: Unable to resolve constant `FIELD`
String::Nested # error: Unable to resolve constant `Nested`
String::FIELD # error: Unable to resolve constant `FIELD`
Thread::Mutex.new.alias_recovery_marker # error: Method `alias_recovery_marker` does not exist
class Consumer
  include B::Mixin
end
Consumer.new.alias_recovery_marker # error: Method `alias_recovery_marker` does not exist
