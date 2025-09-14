# typed: true
# enable-deprecated: true

class TestClass
  extend T::Sig

  sig { deprecated.returns(String) }
  def old_method
    "legacy"
  end

  sig { returns(String) }
  def new_method
    "current"  
  end
end

# This should generate a diagnostic with deprecated tag
obj = TestClass.new
result = obj.old_method
#            ^^^^^^^^^^ hint: Method `TestClass#old_method` is deprecated
puts result

# This should not generate any diagnostic
result2 = obj.new_method
puts result2
