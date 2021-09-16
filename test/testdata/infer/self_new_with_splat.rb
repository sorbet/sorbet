# typed: strict
class Foo
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.works_correctly
    new
  end

  sig {returns(T.attached_class)}
  def self.also_works
    new(*[])
  end

  sig {returns(T.attached_class)}
  def self.also_works_with_block
    new(*[]) do
      1
    end
  end
end
