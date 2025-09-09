# typed: strict

require_relative '../../../gems/sorbet-runtime/lib/sorbet-runtime'

require 'minitest/autorun'
require 'minitest/spec'

# -- acts as a pseudo RBI file, while also being a no-op when run -----
module Minitest; end
class Minitest::Runnable; end
class Minitest::Test < Minitest::Runnable; end
class Minitest::Spec < Minitest::Test
  module DSL
    if T.unsafe(false)
      def let(name, &block); end # error: does not have a `sig`
    end
  end
  extend DSL

  def self.test_each(iter, &blk) # error: does not have a `sig`
    iter.each(&blk)
  end
end
# ---------------------------------------------------------------------


class MyTestHelper < Minitest::Spec
  extend T::Sig

  let(:defined_outside) {
    puts('still works')
  }

  describe 'test' do
    let(:untyped_helper) {
  # ^^^^^^^^^^^^^^^^^^^ error: does not have a `sig`
      'hello'
    }

    sig { returns(String) }
    let(:let_with_sig) {
      'world'
    }

    sig { returns(Integer) }
    let(:let_with_bad_sig) {
      'not an int' # error: Expected `Integer` but found
    }

    sig { returns(Integer) }
    let(:has_constant_definitions) {
      X = T.let(1, Integer)
      X
    }

    it 'example' do
      res = untyped_helper()
      T.reveal_type(res) # error: `T.untyped`
      puts(res)

      res = let_with_sig()
      T.reveal_type(res) # error: `String`
      puts(res)

      begin
        res = let_with_bad_sig()
        T.reveal_type(res) # error: `Integer`
      rescue TypeError => ex
        puts(ex.message)
      end
    end

    it 'counter examples' do
      # This will run ok but Sorbet rejects it, in case there are other `let` DSLs
      # (Sorbet has always rejected it--the recent additions to support `let` are strictly additive)
      begin
        defined_outside() # error: Method `defined_outside` does not exist
      rescue NameError => ex
        puts(ex.message)
      end
    end

    test_each(['once']) do |once|
      it 'inside each' do
        res = let_with_sig
        T.reveal_type(res) # error: `String`
        puts(once)
      end
    end
  end

  test_each(['foo', 'bar']) do |test_case|
    describe("for #{test_case}") do
      sig { returns(NilClass) } # error: Only valid `it`, `before`, `after`, and `describe` blocks can appear within `test_each`
      let(:another_helper) { puts('another') }
      it 'does the thing' do
        begin
          # This one is from the other describe block, not ours
          let_with_sig # error: Method `let_with_sig` does not exist
        rescue NameError => ex
          puts(ex)
        end

        res = another_helper
        T.reveal_type(res) # error: `NilClass`

        begin
          # `let`-defined methods which are defined outside describe are not supported (out of caution in rewriter)
          defined_outside # error: Method `defined_outside` does not exist
        rescue NameError => ex
          puts(ex)
        end
      end
    end
  end
end
