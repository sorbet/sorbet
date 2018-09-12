# typed: strict

class Object 
end

class Trigger
  def trigger # error: does not have a `sig`
    @__fake_logger # error: undeclared variable
  end
end

class Foo < Bar
end
class Bar < Foo # error: Circular dependency
end
