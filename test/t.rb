class T
  def self.type(fixed: nil)
  end
  def self.assert_type!(*args)
  end
end
class T::Sig
  def returns(*args)
  end
end
def sig(*args)
  T::Sig.new
end

class Module
  def [](*args)
    self
  end
end
