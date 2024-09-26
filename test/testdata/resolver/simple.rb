# typed: true
class Outer1
end

class Outer2
  class C
  end

  class Inner1
    class A
    end

    class Inner2
      A
      Inner1
      Inner1::Inner2
      Outer1
    end
  end

  class Inner1::Inner2
    C
    A # error: Unable to resolve constant
  end

  class Other < Inner1::Inner2
  end
end

(Does::Not::Exist)
#^^^^ error: Unable to resolve constant `Does`
