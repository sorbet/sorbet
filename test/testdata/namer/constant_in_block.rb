# typed: true
class A
  
  def self.foo
    foo do
    C = 1 # this is parse error in Ruby, but not in Sorbet
    C # error: Unable to resolve constant
    end
  end
  
  [].each do
    B = 1
    B
  end
end
