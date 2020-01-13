# typed: true
# compiled: true
A = 1
puts A

class B
  A = 2
  puts A
end
puts B::A
