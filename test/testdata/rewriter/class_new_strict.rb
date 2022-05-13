# typed: strict
class A
  extend T::Sig
  sig {void}
  def self.make
    cls = Class.new(A) do
    end
  end
end
