# typed: strict
extend T::Sig

class MyEnum
  extend T::Helpers
  abstract!
  sealed!

  class X < MyEnum; include Singleton; final!; end
  class Y < MyEnum; include Singleton; final!; end
end

sig {params(x: MyEnum).returns(Integer)}
def example_with_class_objects(x)
  res = case x
  when MyEnum::X then 1
  when MyEnum::Y then 2
  end

  res # no error (cases are exhaustive so res is not nilable)
end
