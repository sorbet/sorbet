# typed: true

# Metrics:
#  * 1 module     M
#  * 1 class      T.class_of(M)
#  * 1 singleton  T.class_of(M)
#  * 3 methods    <root>.static_init, M1.foo, M1.static_init

module M1
  def self.foo; end
end
