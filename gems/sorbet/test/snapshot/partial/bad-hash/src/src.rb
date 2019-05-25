# typed: true
class A
  def hash
    "whoops"
  end
end

class B
  C = A.new
  D = C
end
