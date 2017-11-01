# error:
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
    A # unresolved
  end

  class Other < Inner1::Inner2
  end
end
