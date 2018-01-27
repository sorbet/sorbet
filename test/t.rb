class T
  def self.type
  end
  def self.assert_type!(*args)
  end
  def self.type_alias(*args)
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
