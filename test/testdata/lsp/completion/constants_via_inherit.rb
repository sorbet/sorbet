# typed: true

class Parent1
  XXX = 1
end
class Child1 < Parent1
  XX # error: Unable to resolve
  # ^ completion: XXX
end

class Parent2
  class ParentInner2
    YYY = 1
  end
end
class Child2 < Parent2
  class ChildInner2 < ParentInner2
    YY # error: Unable to resolve
    # ^ completion: YYY
  end
end

class OuterParent3
  ZZZ = nil
end
class OuterChild3 < OuterParent3
  class Inner
    ZZ # error: Unable to resolve
    # ^ completion: (nothing)

    OuterParent3::ZZ # error: Unable to resolve constant
    #               ^ completion: ZZZ
  end
end
