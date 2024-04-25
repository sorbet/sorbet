# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './lambda_return_compiled_interpreted__2'

puts F0.call(100)
puts M::F1.call(200)
puts C::F2.call(400)
puts C.f3.call(300)
puts C.new.f4.call(500)
puts F5.call.call(600)
puts F8.call(449,451)
