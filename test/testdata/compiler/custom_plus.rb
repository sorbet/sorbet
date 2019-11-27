# typed: true
# compiled: true

class A
  def +(b)
    return self
  end
end

a = A.new

def delegate(a)
  if a + 1 == a
    puts "ok"
  else 
    puts "fail"
  end
end

delegate(a)
