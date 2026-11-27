# typed: true

class A
  private :example1

  def example2; end
end

A.new.example1 # error: Non-private call to private method `example1` on `A`
A.new.example2 # error: Non-private call to private method `example2` on `A`
