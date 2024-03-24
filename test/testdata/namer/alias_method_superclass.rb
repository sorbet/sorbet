# typed: true

class Super
  extend T::Sig
  sig {params(x: Integer).returns(String)}
  def parent_method(x)
    x.to_s
  end
end

class A < Super
  alias_method(:bar, :parent_method)
end

A.new.parent_method(1)
A.new.bar(1)


module SomeModuleExtension
  def extension_method
  end
end

class SomeExtendedClass
  include SomeModuleExtension
  alias_method :aliased_method, :extension_method
end
