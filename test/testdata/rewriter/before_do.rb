# typed: strict
class Module; include T::Sig; end

class MyTestGrandParent
  sig {params(name: String).void}
  def initialize(name)
  end
end

class MyTestParent < MyTestGrandParent
  before do
    @x = T.let(1, Integer)
  end
end

class MyTestChild < MyTestParent
  sig {params(name: String).void}
  def initialize(name)
    super
  end

  describe 'example' do
    it 'does thing' do
      T.reveal_type(@x) # error: `Integer`
    end
  end
end
