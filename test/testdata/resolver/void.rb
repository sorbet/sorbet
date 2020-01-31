# typed: true

puts T::Private::Types::Void::VOID

# Ensures that VOID gets entered as a class symbol (can be used in type position)
T.cast(nil, T::Private::Types::Void::VOID)
