module A
  B::C = 1

  module B
    C

    D = 1
  end
end

A::B::C
A::B::D
::A
