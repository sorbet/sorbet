# typed: true

class RSpecLetSubjectTest
  extend T::Sig

  # Test let functionality
  describe 'let blocks' do
    let(:helper_method) { 'hello world' }
    
    sig { returns(String) }
    let(:typed_helper) { 'typed string' }

    it 'can use let methods' do
      result = helper_method
      puts result
      
      typed_result = typed_helper
      T.reveal_type(typed_result) # error: `String`
      puts typed_result
    end
  end

  # Test let! functionality  
  describe 'let! blocks' do
    let!(:eager_helper) { 'evaluated immediately' }
    
    sig { returns(Integer) }
    let!(:eager_number) { 42 }

    it 'can use let! methods' do
      result = eager_helper
      puts result
      
      number = eager_number
      T.reveal_type(number) # error: `Integer`
      puts number
    end
  end

  # Test subject functionality
  describe 'subject blocks' do
    subject(:my_subject) { 'subject value' }
    
    it 'can use named subject' do
      result = my_subject
      puts result
    end

    context 'with anonymous subject' do
      subject { 'anonymous subject' }
      
      it 'can use anonymous subject' do
        result = subject
        puts result
      end
    end
  end

  # Test mixed let, let!, and subject
  describe 'mixed let variants' do
    let(:regular_let) { 'regular' }
    let!(:eager_let) { 'eager' }
    subject(:main_subject) { 'subject' }
    
    it 'can use all variants together' do
      puts regular_let
      puts eager_let  
      puts main_subject
    end
  end
end
