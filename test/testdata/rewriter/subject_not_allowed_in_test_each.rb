# typed: true

class SubjectNotAllowedInTestEach
  extend T::Sig

  def self.test_each(iter, &blk); end
  test_each(['foo', 'bar']) do |test_case|
    describe("subject rejection test for #{test_case}") do
      # subject blocks are allowed in test_each when inside describe
      subject { "allowed" }

      it 'should not be able to use subject' do
        puts "test"
      end
    end
  end
end