# typed: true

class A
  attr_reader :using_attr_reader
end

class B < T::Struct
  prop :using_t_struct_prop, Integer
  const :using_t_struct_const, Integer
end

class C
  def using_bare_method
    @using_bare_method
  end
end

class D
  def self.using_bare_self_method
    @using_bare_self_method
  end
end
