# typed: true

class SubjectNotAllowedInTestEach
  extend T::Sig

  test_each(['foo', 'bar']) do |test_case|
    describe("subject rejection test for #{test_case}") do
      # This should produce an error: subject blocks are not allowed in test_each
      subject { "not allowed" }
      # ^^ error: Only valid `it`, `before`, `after`, and `describe` blocks can appear within `test_each`

      it 'should not be able to use subject' do
        puts "test"
      end
    end
  end
end