# typed: true
module D 
  D=1
  class D::D; end
      # ^^^^ error: Can't nest `D` under `D::D` because `D::D` is not a class or module
end

