# frozen_string_literal: true
# typed: true
# compiled: false

# TODO(aprocter): This file is interpreted, lambda_return__2 is compiled. Want to try other permutations as well (both
# compiled, and only this file compiled).
require_relative './lambda_return__2'

puts F0.call(100)
puts M::F1.call(200)
puts C::F2.call(400)
puts C.f3.call(300)
puts C.new.f4.call(500)
puts F5.call.call(600)
puts F6.call.call(700)
puts F7.call.call(800)
puts F8.call(449,451)
