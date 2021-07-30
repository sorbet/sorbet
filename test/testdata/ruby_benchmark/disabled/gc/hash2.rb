# frozen_string_literal: true
# typed: true
# compiled: true
value = 0.01
h = {}
n = 4*(10**6)

1.upto(n){|i|
  h["%020d" % i] = value * i
}
