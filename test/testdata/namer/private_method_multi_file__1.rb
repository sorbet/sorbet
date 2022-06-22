# typed: true

# This test captures buggy behavior. Ideally, example1 and example2 would be
# treated the same. The fact that one is private and the other isn't reflects a
# dependency on the order of files given to Sorbet, which should not be
# meaningful.

class A
  def example1; end

  private :example2
end

A.new.example1 # error: Non-private call to private method `example1` on `A`
A.new.example2
