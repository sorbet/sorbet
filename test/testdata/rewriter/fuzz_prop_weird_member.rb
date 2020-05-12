# typed: false
prop :a, ::Array # error: `T.non_forcing_is_a?` can only resolve strings to constants in `# typed: true` files (or higher)
