# typed: true
module Base
  def f(x); end
end
module M1
  include Base
  def f(x, y); end
end
module M2
  include Base
end

class C
  include M1
  include M2
end

C.new.f(1, 2)
