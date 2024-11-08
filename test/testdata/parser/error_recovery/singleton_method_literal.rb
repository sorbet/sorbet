# typed: true

def (10).test()
   # ^^ error: cannot define a singleton method for a literal
  [1,2,3].each do |x|
    x
  end
end
