# typed: true
module D
  D=1 # error: Cannot initialize the class or module `D` by constant assignment
  class D::D; end

end
