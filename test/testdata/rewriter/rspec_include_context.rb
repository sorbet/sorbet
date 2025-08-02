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

      it 'can use test assertions within shared examples' do
        expect(shared_variable).to eq('shared value')
      end
    end

    include_context 'user helpers'
    
    it 'can access variable from included context' do
      puts shared_variable
    end
  end
end

# Test shared_context within RSpec.describe (proper RSpec context)
# Define minimal RSpec for testing
module RSpec
  module Core
    class ExampleGroup
      def self.describe(name, &block); end
      def expect(*args); end
      def eq(*args); end
    end
  end
  
  def self.describe(name, &block)
    Core::ExampleGroup.describe(name, &block)
  end
end

RSpec.describe 'RSpec context with shared_context' do
  shared_context 'authenticated user' do
    let(:user) { 'authenticated_user' }
    let(:token) { 'auth_token_123' }
    let(:auth_helper) { "#{user} with #{token}" }
  end

  describe 'API endpoint' do
    include_context 'authenticated user'
    
    it 'has access to shared context variables and methods' do
      expect(user).to eq('authenticated_user')
      expect(token).to eq('auth_token_123') 
      expect(auth_helper).to eq('authenticated_user with auth_token_123')
    end
  end
end
