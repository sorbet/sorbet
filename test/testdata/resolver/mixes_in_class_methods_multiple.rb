# typed: true
module Mixin
  extend T::Helpers

  module ClassMethods1
    def mixin_class_method_1
    end
  end

  module ClassMethods2
    def mixin_class_method_2
    end
  end

  module ClassMethods3
    def mixin_class_method_3
    end
  end

  mixes_in_class_methods(ClassMethods1, ClassMethods2, ClassMethods3)
 
  def mixin_method
  end
end

class Test
  include Mixin
end

Test.mixin_class_method_1
Test.mixin_class_method_2
Test.mixin_class_method_3
Test.new.mixin_method

# Test the modification of ancestor chain ordering
# Uses different number of arguments to identify which method was called
module Mixin2
  extend T::Helpers

  module ClassMethods1
    def foo
    end
  end

  module ClassMethods2
    def foo(a)
    end
  end

  mixes_in_class_methods(ClassMethods1, ClassMethods2)

  module ClassMethods3
    def foo(a, b, c)
    end
  end

  module ClassMethods4
    def foo(a, b, c, d)
    end
  end

  mixes_in_class_methods(ClassMethods3, ClassMethods4)
end

class Test2
  include Mixin2
end

Test2.foo # error: Not enough arguments provided for method `Mixin2::ClassMethods4#foo`. Expected: `4`, got: `0`
