# typed: true

class RSpecItsTest
  extend T::Sig

  # Test its functionality
  describe 'its functionality' do
    subject { { name: 'John', age: 30 } }
    
    its(:name) { puts 'testing name property' }
    its(:age) { puts 'testing age property' }
    its(:size) { puts 'testing size method' }
  end

  # Test its with different subjects
  describe 'its with string subject' do  
    subject { 'hello world' }
    
    its(:length) { puts 'testing string length' }
    its(:upcase) { puts 'testing string upcase' }
  end

  # Test its with array subject
  describe 'its with array subject' do
    subject { [1, 2, 3, 4, 5] }
    
    its(:first) { puts 'testing array first' }
    its(:last) { puts 'testing array last' }
    its(:count) { puts 'testing array count' }
  end

  # Test nested its
  describe 'nested context with its' do
    subject { { user: { profile: { name: 'Jane' } } } }
    
    context 'accessing nested properties' do
      its(:user) { puts 'accessing user hash' }
      
      context 'deeper nesting' do
        subject { super()[:user] }
        its(:profile) { puts 'accessing profile' }
      end
    end
  end
end