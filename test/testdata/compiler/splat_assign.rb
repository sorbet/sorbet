# frozen_string_literal: true
# typed: true
# compiled: true

a,*,b,c = [1,2,3,4,5,6,7]
puts [a,b,c]

a,*,b,c = [1,2]
puts [a,b,c]

a,*,b,c = []
puts [a,b,c]
