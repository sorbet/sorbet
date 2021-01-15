# typed: true

class A
  attr_reader :using_attr_reader
end

class B < T::Struct
  prop :using_t_struct_prop, Integer
  const :using_t_struct_const, Integer
end

class C < T::Struct
  def using_bare_method
    @using_bare_method
  end
end
