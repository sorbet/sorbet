# typed: strict
class A
  extend T::Sig
  sig {void}
  def self.make
    _cls = Class.new do
    end

    _cls = Class.new(A) do
    end
  end
end
