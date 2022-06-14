# typed: true

# Metrics:
#  * 0 module
#  * 2 classes    C1, T.class_of(C1)
#  * 1 singleton  T.class_of(C1)
#  * 3 methods    <root>.static_init, C1#foo, C1.static_init

class C1
  def foo; end
end
