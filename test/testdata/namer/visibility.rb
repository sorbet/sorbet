# typed: true

class A
  def f1; end
  public def f2; end
  private def f3; end
  protected def f4; end
  private_class_method def self.f5; end
end

class B
  def f1; end
  def f2; end
  def f3; end
  def f4; end
  def self.f5; end
  public :f2
  private :f3
  protected :f4
  private_class_method :f5
end
