# typed: strict

class A
  extend T::Sig
  sig {void}
  def outer
    sig {void}
    def inner; end
  end
end

A.outer # error: Method `outer` does not exist on `T.class_of(A)`
A.new.outer
A.inner # error: Method `inner` does not exist on `T.class_of(A)`
A.new.inner

class B
  extend T::Sig
  sig {void}
  def outer
    sig {void}
    def self.inner; end
  end
end

B.outer # error: Method `outer` does not exist on `T.class_of(B)`
B.new.outer
B.inner # error: Method `inner` does not exist on `T.class_of(B)`
B.new.inner

class C
  extend T::Sig
  sig {void}
  def self.outer
    sig {void}
    def inner; end
  end
end

C.outer
C.new.outer # error: Method `outer` does not exist on `C`
C.inner # error: Method `inner` does not exist on `T.class_of(C)`
C.new.inner

class D
  extend T::Sig
  sig {void}
  def self.outer
    sig {void}
    def self.inner; end
  end
end

D.outer
D.new.outer # error: Method `outer` does not exist on `D`
D.inner
D.new.inner # error: Method `inner` does not exist on `D`
