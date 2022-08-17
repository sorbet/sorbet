# typed: true
A = 91
class A # error: Redefining constant `A` as a class or module
end

class B
end
B = 91 # error: Redefining constant `B` as a static field
