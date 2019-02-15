# typed: true
module D 
  D=1
  class D::D; end
      # ^^^^ error: Nesting is only permitted inside classes and modules
# ^^^^^^^^^^^^^^^ error: was previously defined
end

