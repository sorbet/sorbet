# typed: false

# Test that incomplete/malformed syntax with 'it' parameter doesn't segfault or produce confusing errors
# Note: Tests for mixing 'it' with numbered params or explicit args are in separate files

# Case 1: incomplete expression with 'it'
[1, 2, 3].map { it + } # error: missing arg to "+" operator

# Case 2: incomplete block expression
[1, 2].map { it. } # error: unexpected token tRCURLY

# Case 3: double negation operator with incomplete expression
foo do
  puts it
  !! # error: missing arg to "!" operator
end
