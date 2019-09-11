# typed: true
module M1 # linearization: M1
  def foo; 1; end;
end

module M2 # linearization: M1
  def foo; 2; end;
end

module M3 # linearization: M3, M2, M1
  include M1
  include M2
end
class A # linearization: A, M3, M2, M1
  include M2
  include M3
end
A.new.foo # => 2

class B
  include M2, M3
end
