# typed: true
# Spacer so error assertions are on same line

A.new(0)
A.new('') # error: Expected `Integer` but found `String("")`
