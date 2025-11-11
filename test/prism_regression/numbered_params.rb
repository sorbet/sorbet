# typed: false

proc { _1 + _2 }
proc do _1 + _2 end
# -> { _1 + _2 } # TODO: Fix lambda location errors

# Use only a high numbered parameter, without using any of the lower numbers.
# * Legacy parse tree: only stores decls of NumParams that were actually encountered in the body.
# * Desugar tree: always declares all params (`_1, _2, ..., _9`)
proc { _9 }
proc do _9 end
# -> { _9 } # TODO: Fix lambda location errors

# Use the numbered parameters out-of-order
# * Legacy parse tree: stores NumParams in the order they were encountered in the body.
# * Desugar tree: stores all params in numbered order.
proc { _2 + _1 }
proc { _2 + _1 }
# -> { _2 + _1 } # TODO: Fix lambda location errors
