# typed: true

class ClassA
  it 'first_test' do
    describe 'BAR' do
    end
  end

  describe 'OUTER' do
    it 'second_test' do
      describe 'QUX' do
      end
    end
  end
end

module ModuleB
  it 'first_test' do
    describe 'BAR' do
    end
  end

  describe 'OUTER' do
    it 'second_test' do
      describe 'QUX' do
      end
    end
  end
end
