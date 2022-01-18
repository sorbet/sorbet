# compiled: true
# typed: true

class A
  extend T::Sig

  attr_reader :using_attr_reader

  sig {returns(Integer)}
  attr_reader :using_attr_reader_sig_checked

  sig {returns(Integer).checked(:tests)}
  attr_reader :using_attr_reader_sig_checked_tests

  sig {returns(Integer).checked(:never)}
  attr_reader :using_attr_reader_sig_checked_never
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
