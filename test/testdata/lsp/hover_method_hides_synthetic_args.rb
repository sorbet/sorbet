# typed: true
class A
  extend T::Sig\
  
  sig {params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end

def main
  A.new.bar(10)
       # ^ hover: sig {params(x: Integer).returns(String)}
end
