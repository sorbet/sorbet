# typed: true

def (10).test()
   # ^^ error: cannot define a singleton method for a literal
   # ^^ error: `def EXPRESSION.method` is only supported for `def self.method`
  [1,2,3].each do |x|
    x
  end
end
