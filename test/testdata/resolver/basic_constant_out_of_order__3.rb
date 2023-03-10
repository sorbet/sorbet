# typed: strict

module Bar
  p(Baz)

  class Baz::A
    p(Baz)
  end

  class Baz::B
  end
end
