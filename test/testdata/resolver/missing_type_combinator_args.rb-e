# typed: true
T.cast(nil, T.nilable) # error: Not enough arguments provided for method
T.cast(nil, T.nilable(Integer, Integer)) # error: Too many arguments provided for method
T.cast(nil, T.any) # error: Not enough arguments provided for method
T.cast(nil, T.all) # error: Not enough arguments provided for method
T.must # error: Not enough arguments provided for method
T.let(1) # error: Not enough arguments provided for method
T.let(1, 2, 3) # error: Unsupported type syntax
T.reveal_type() # error: Not enough arguments provided for method
T.reveal_type(1, 2) # error: Too many arguments provided for method
