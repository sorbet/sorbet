# typed: true

class A
  def example1; end

  private :example2
end

A.new.example1 # error: Non-private call to private method `example1` on `A`
A.new.example2 # error: Non-private call to private method `example2` on `A`
