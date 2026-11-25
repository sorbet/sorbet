# typed: false

# Regular assignment
self.m = 1

# Compound assignment operators
self.m  &= 2
self.m  |= 3
self.m  ^= 4
self.m >>= 5
self.m <<= 6
self.m  += 7
self.m  -= 8
self.m  %= 9
self.m  /= 10
self.m  *= 11
self.m **= 12

# Special cases
self.m &&= 13
self.m ||= 14

# Multi-target assignment
self.target1, self.target2 = 15, 16

FOO.target1, FOO.target2 = 17, 18 # `isPrivateOk` should be false
