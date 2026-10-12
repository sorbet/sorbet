# typed: true

# Reading through aliases remains valid.
T.let(A::Same.new, A::Thing)
#             ^^^ error: Method `new` does not exist on `T.class_of(A::Same)`
T.let(A::Foreign.new, B::Thing)
#                ^^^ error: Method `new` does not exist on `T.class_of(A::Foreign)`
T.let(A::Chained.new, B::Thing)
#                ^^^ error: Method `new` does not exist on `T.class_of(A::Chained)`
T.let(A::Unpackaged.new, String)
#                   ^^^ error: Method `new` does not exist on `T.class_of(A::Unpackaged)`
# Rejected definitions must leave the alias targets unchanged.
A::Thing::Nested # error: Unable to resolve constant `Nested`
A::Thing::FIELD # error: Unable to resolve constant `FIELD`
B::Thing::Nested # error: Unable to resolve constant `Nested`
B::Thing::FIELD # error: Unable to resolve constant `FIELD`
String::Nested # error: Unable to resolve constant `Nested`
String::FIELD # error: Unable to resolve constant `FIELD`
Thread::Mutex.new.alias_recovery_marker
#                 ^^^^^^^^^^^^^^^^^^^^^ error: Method `alias_recovery_marker` does not exist on `Thread::Mutex`
class Consumer
  include B::Mixin
end
Consumer.new.alias_recovery_marker
#            ^^^^^^^^^^^^^^^^^^^^^ error: Method `alias_recovery_marker` does not exist on `Consumer`
