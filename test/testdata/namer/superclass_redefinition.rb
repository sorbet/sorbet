# typed: strict
class A < Object
end
class B
end

class A < B # error: parents redefined
end
