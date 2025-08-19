# typed: true

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

RSpec.describe 'include context' do
  describe 'nonexistent context' do
    it 'cannot reference undefined context' do
      include_context 'nonexistent context' # error: Unable to resolve constant `<shared_examples_module 'nonexistent context'>
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

  describe 'RSpec context with shared_context' do
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

    # Test include_context with parameters (should ignore parameters)
    describe 'e2e context' do
      shared_context 'e2e context' do
        let(:foo) { 'bar' }
        let(:config) { 'default_config' }
      end

      describe 'with parameters' do
        include_context 'e2e context', { foo: 'bar' }
        
        it 'should include the context and ignore parameters' do
          expect(foo).to eq('bar')
          expect(config).to eq('default_config')
        end
      end
    end
  end
end
