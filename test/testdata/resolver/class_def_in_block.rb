# typed: strict

class A
  [].each do
    class B; end
  end

  @x = T.let(0, Integer)
end
