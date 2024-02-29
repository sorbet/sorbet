# typed: true

x = T.unsafe(nil)

T::NonForcingConstants.non_forcing_is_a? # error: Not enough arguments

p T::NonForcingConstants.non_forcing_is_a?(nil, nil)
#                                               ^^^ error: Expected `String`
#                                               ^^^  error: only accepts string literals

T::NonForcingConstants.non_forcing_is_a?(x, '') # error: must not be empty

T::NonForcingConstants.non_forcing_is_a?(x, '::') # error: Unable to resolve constant `::`
T::NonForcingConstants.non_forcing_is_a?(x, ':::') # error: Unable to resolve constant `:::`
T::NonForcingConstants.non_forcing_is_a?(x, '::::') # error: Unable to resolve constant `::`

T::NonForcingConstants.non_forcing_is_a?(x, 'X') # error: must be an absolute constant reference

T::NonForcingConstants.non_forcing_is_a?(x, '::DoesntExist') # error: Unable to resolve constant `::DoesntExist`
T::NonForcingConstants.non_forcing_is_a?(x, '::Typo:Typo') # error: Unable to resolve constant `::Typo:Typo`
T::NonForcingConstants.non_forcing_is_a?(x, '::Doesnt::Exist') # error: Unable to resolve constant `::Doesnt`

A = 1
T::NonForcingConstants.non_forcing_is_a?(0, '::A') # error: must resolve to a class or module

p Integer
T::NonForcingConstants.non_forcing_is_a?(x, '::Integer')

class Outer
  class Nested
    class Inner; end
  end

  N = Nested
end

p ::Outer::N::Inner
x = T::NonForcingConstants.non_forcing_is_a?(Outer::Nested::Inner.new, '::Outer::N::Inner')
T.reveal_type(x) # error: Revealed type: `T::Boolean`

# Prints first symbol to not resolve when name doesn't exist
T::NonForcingConstants.non_forcing_is_a?(x, '::Integer::DoesntExist::After') # error: Unable to resolve constant `::Integer::DoesntExist`
# Prints first symbol to not resolve when name exists but symbol doesn't
T::NonForcingConstants.non_forcing_is_a?(x, '::Integer::String::After') # error: Unable to resolve constant `::Integer::String`

T.reveal_type(T::NonForcingConstants.non_forcing_is_a?('', '::String')) # error: Revealed type: `T::Boolean`
T.reveal_type(T::NonForcingConstants.non_forcing_is_a?(0, '::String')) # error: Revealed type: `T::Boolean`

i_or_s = T.let(0, T.any(Integer, String))
T.reveal_type(T::NonForcingConstants.non_forcing_is_a?(i_or_s, '::String')) # error: Revealed type: `T::Boolean`
if T::NonForcingConstants.non_forcing_is_a?(i_or_s, '::String')
  puts i_or_s # ensure reachable

  # TODO(jez) Consider changing updateKnowledge to teach Sorbet that this is for sure type `String`
  T.reveal_type(i_or_s) # error: Revealed type: `T.any(Integer, String)`
end

oni = T.unsafe(Outer::Nested::Inner.new)

# these, that reference non-existing constants, should all yield the same error
T::NonForcingConstants.non_forcing_is_a?(oni, "::Outer::Nested::Other") # error: Unable to resolve constant `::Outer::Nested::Other`
