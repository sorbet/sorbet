# typed: false

foo1(1, 2) {} # inline block arg

foo2(1, 2) do # do-end block arg
end

# TODO: Fix block pass argument locations
# foo3(1, 2, &FORWARDED_BLOCK) # block pass argument

### Test calls with no `closing_loc`

# This variation is a syntax error.
# TODO: handle this correctly.
# Prism reports: "unexpected '{' after a method call without parenthesis"
# foo4 1, 2 {} # inline block arg

foo5 1, 2 do # do-end block arg
end

# TODO: Fix block pass argument locations
# foo6 1, 2, &FORWARDED_BLOCK # block pass argument
