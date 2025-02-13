# typed: true
extend T::Sig

module IFoo; end;

sig { params(klass: T::Class[IFoo]).void }
def takes_t_class_ifoo(klass)
end

takes_t_class_ifoo(IFoo)

module IFooWithTT
  extend T::Generic
  X = type_template(:out) {{fixed: Integer}}
end;

sig { params(klass: T::Class[IFooWithTT]).void }
def takes_t_class_ifoo_with_tm(klass)
end

takes_t_class_ifoo_with_tm(IFooWithTT)
