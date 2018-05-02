# rubocop:disable PrisonGuard/NoTopLevelDeclarations
class T
  def self.type(fixed: nil); end
  def self.assert_type!(*args); end
  def self.untyped; end
end

class T::Sig
  def returns(*args); end
end

def sig(*_args)
  T::Sig.new
end

class Module
  def [](*_args)
    self
  end
end
# rubocop:enable PrisonGuard/NoTopLevelDeclarations
