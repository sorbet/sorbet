# typed: true

class RSpecIncludeContextTest
  extend T::Sig

  # NOTE: I cannot get this test to pass – but I know this is working "in production" because
  # it works on the Rails monolith this was tested against.
  describe 'nonexistent context' do
    it 'cannot reference undefined context' do
      include_context 'nonexistent context' # error: Unable to resolve constant `<shared_examples 'nonexistent context'>`
    end
  end

  describe 'working include_context' do
    shared_examples 'user helpers' do
      let(:shared_variable) { 'shared value' }
    end

    include_context 'user helpers'
    
    it 'can access variable from included context' do
      puts shared_variable
    end
  end
end
