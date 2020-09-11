# typed: true
module Mixin
  extend T::Sig

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