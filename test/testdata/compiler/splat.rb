# frozen_string_literal: true
# typed: true
# compiled: true

# Something that doesn't support to_a
x = *1
p x

# Something that does support to_a
x = *{foo: true}
p x

x = *[1, 2, 3]
p x
