# typed: false

T::NonForcingConstants.non_forcing_is_a?('::Integer') # error: in `# typed: true` files
T::NonForcingConstants.non_forcing_is_a?('::DoesntExist') # error: in `# typed: true` files
