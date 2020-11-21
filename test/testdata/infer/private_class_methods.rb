# typed: true

# By these methods are defined on Object
def self.bar; end

class Test
  def self.method_a; end
  def self.method_b; end
  private_class_method :method_b
  private_class_method def self.method_c; end

  class << self
    def method_d; end
    private :method_d

    private def method_e; end

    private

    def method_f; end
  end

  module ClassMethods
    def method_g; end
    private :method_g

    private def method_h; end

    private

    def method_i; end
  end

  extend ClassMethods
end

Object.bar # error: Non-private call to private method `bar` on `T.class_of(Object)`

Test.method_a
Test.method_b # error: Non-private call to private method `method_b` on `T.class_of(Test)`
Test.method_c # error: Non-private call to private method `method_c` on `T.class_of(Test)`
Test.method_d # error: Non-private call to private method `method_d` on `T.class_of(Test)`
Test.method_e # error: Non-private call to private method `method_e` on `T.class_of(Test)`
Test.method_g # error: Non-private call to private method `method_g` on `T.class_of(Test)`
Test.method_h # error: Non-private call to private method `method_h` on `T.class_of(Test)`

# TODO: The following methods should contain errors. Sorbet currently does not support setting method
# visibility using the private/protected keywords that affect the visibility of subsequent methods.
Test.method_f
Test.method_i
