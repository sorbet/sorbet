# typed: false

# Test that mixing 'it' with numbered parameters produces parser errors

# Using 'it' after _1 should error
[1, 2, 3].map { _1 + it } # error: 'it' is not allowed when a numbered parameter is already used

# Using _1 after 'it' should error
[1, 2, 3].map { it + _1 } # error: numbered parameters are not allowed when 'it' is already used
