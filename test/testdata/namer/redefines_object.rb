# @typed

class Object 
end

class Trigger
  def trigger
    @__fake_logger # error: undeclared variable
  end
end

class Foo < Bar
end
class Bar < Foo # error: Circular dependency
end
