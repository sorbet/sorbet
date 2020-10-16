# typed: true

x = T.unsafe(nil)

T::NonForcingConstants.non_forcing_inherits_from? # error: Not enough arguments

T::NonForcingConstants.non_forcing_inherits_from?(Integer, nil)
#                                                          ^^^ error: Expected `String`
#                                                          ^^^  error: only accepts string literals

T::NonForcingConstants.non_forcing_inherits_from?(1, '::Integer')
#                                                 ^ error: Expected `Module` but found `Integer(1)` for argument `val`

T::NonForcingConstants.non_forcing_inherits_from?(x, '') # error: must not be empty

T::NonForcingConstants.non_forcing_inherits_from?(x, '::') # error: Unable to resolve constant `::`
T::NonForcingConstants.non_forcing_inherits_from?(x, ':::') # error: Unable to resolve constant `:::`
T::NonForcingConstants.non_forcing_inherits_from?(x, '::::') # error: Unable to resolve constant `::`

T::NonForcingConstants.non_forcing_inherits_from?(x, 'X') # error: must be an absolute constant reference

T::NonForcingConstants.non_forcing_inherits_from?(x, '::DoesntExist') # error: Unable to resolve constant `::DoesntExist`
T::NonForcingConstants.non_forcing_inherits_from?(x, '::Typo:Typo') # error: Unable to resolve constant `::Typo:Typo`
T::NonForcingConstants.non_forcing_inherits_from?(x, '::Doesnt::Exist') # error: Unable to resolve constant `::Doesnt`


class Outer
  class Inner; end
end

class InnerChild < Outer::Inner; end


x = T::NonForcingConstants.non_forcing_inherits_from?(Integer, '::Numeric')
T.reveal_type(x) # error: Revealed type: `T::Boolean`

inner_child_klass = InnerChild
T::NonForcingConstants.non_forcing_inherits_from?(inner_child_klass, '::Outer::Inner')

T::NonForcingConstants.non_forcing_inherits_from?(InnerChild.new, '::Outer::Inner')
#                                                 ^^^^^^^^^^^^^^ error: Expected `Module` but found `InnerChild` for argument `val`
