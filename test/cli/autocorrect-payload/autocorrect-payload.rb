# typed: true

FOO = 10 # TODO Also fix this autocorrect

a = FOO
[].each { |i| a = nil }

b = Float::INFINITY
[].each { |i| b = nil }

c = 10
[].each { |i| c = nil }
