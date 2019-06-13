# typed: true

extend T::Sig

class Parent; end
class Child < Parent; end
class Other; end

sig {params(x: T.any(Child, Other)).returns(Integer)}
def parent_other(x)
  case x
  when Parent
    1
  when Other
    2
  end
end

sig {params(x: T.any(Child, Other)).returns(Integer)}
def other_parent(x)
  case x
  when Other
    1
  when Parent
    2
  end
end
