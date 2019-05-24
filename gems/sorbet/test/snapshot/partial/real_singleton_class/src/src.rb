class A
  def self.singleton_class
    raise "nope"
  end
end

class B
  C = A.new
end
