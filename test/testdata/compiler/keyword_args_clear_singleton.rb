# frozen_string_literal: true
# typed: true
# compiled: true

# Since we use a global singleton for passing keyword arguments, we need to
# make sure that we completely clear out any keyword arguments from previous
# calls before re-populating the hash for the first call.

p(x: 1, extra: 2)
p(y: 3)
