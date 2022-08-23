# typed: true
A = 91 # error: Cannot initialize the class `A` by constant assignment
class A
end

class B
end
B = 91 # error: Cannot initialize the class `B` by constant assignment
