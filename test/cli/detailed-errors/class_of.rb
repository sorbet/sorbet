# typed: true
extend T::Sig

module IFoo; end;

sig { params(klass: T::Class[IFoo]).void }
def takes_t_class_ifoo(klass)
end

takes_t_class_ifoo(IFoo)
