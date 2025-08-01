# typed: true

class RSpecIncludeContextTest
  extend T::Sig

  shared_examples 'user helpers' do
    let(:shared_variable) { 'shared value' }
  end

  describe 'nonexistent context' do
    it 'cannot reference undefined context' do
      include_context 'nonexistent context' # error: Unable to resolve constant `<shared_examples 'nonexistent context'>`
    end
  end

  describe 'working include_context' do
    include_context 'user helpers'
    
    it 'can access variable from included context' do
      puts shared_variable
    end
  end
end
