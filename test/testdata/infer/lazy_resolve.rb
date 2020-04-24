# typed: true

T.lazy_resolve # error: Not enough arguments

T.lazy_resolve(1) # error: Expected `String` but found `Integer(1)` for argument `klass`

T.lazy_resolve('') # error: must not be empty

T.lazy_resolve('X') # error: must be an absolute constant reference

T.lazy_resolve('::DoesntExist') # error: Unable to resolve constant `::DoesntExist`
T.lazy_resolve('::Typo:Typo') # error: Unable to resolve constant `::Typo:Typo`
T.lazy_resolve('::Doesnt::Exist') # error: Unable to resolve constant `::Doesnt`

p Integer
T.lazy_resolve('::Integer')

class Outer
  class Nested
    Inner = 1
  end

  N = Nested
end

p ::Outer::N::Inner
x = T.lazy_resolve('::Outer::N::Inner')
T.reveal_type(x) # error: Revealed type: `T::Private::LazilyResolvedConstant`

# Prints first symbol to not resolve when name doesn't exist
T.lazy_resolve('::DoesntExist::N::Inner') # error: Unable to resolve constant `::TypoOuter`
# Prints first symbol to not resolve when name exists but symbol doesn't
T.lazy_resolve('::Integer::N::Inner') # error: Unable to resolve constant `::Integer::N`
