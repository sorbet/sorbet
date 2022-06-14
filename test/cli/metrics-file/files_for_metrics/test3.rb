# typed: true

# Metrics:
#  * 0 module
#  * 3 classes    C2, T.class_of(C2), T.class_of(T.class_of(C2))
#  * 2 singleton  T.class_of(C2), T.class_of(T.class_of(C2))
#  * 4 methods    <root>.static_init, C2.static_init, T.class_of(C2)#foo, T.class_of(C2).static_init

class C2
  class << self
    def foo; end
  end
end
