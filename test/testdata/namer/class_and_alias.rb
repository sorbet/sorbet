# typed: strict
A = 91
class A # error: Redefining constant `A`
end

class B
end
B = 91 # error: Redefining constant `B`
