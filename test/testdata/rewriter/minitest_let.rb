# typed: true

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
      def let(name, &block); end
    end
  end
  extend DSL
end
# ---------------------------------------------------------------------


class MyTestHelper < Minitest::Spec
  extend T::Sig

  let(:defined_outside) {
    puts('still works')
  }

  describe 'test' do
    let(:untyped_helper) {
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

    it 'example' do
      res = untyped_helper()
      T.reveal_type(res)
      puts(res)

      res = let_with_sig()
      T.reveal_type(res)
      puts(res)

      begin
        res = let_with_bad_sig()
        T.reveal_type(res) # => `Integer`
      rescue TypeError => ex
        puts(ex.message)
      end
    end

    it 'counter examples' do
      # This will run ok but Sorbet rejects it, in case there are other `let` DSLs
      # (Sorbet has always rejected it--the recent additions to support `let` are strictly additive)
      defined_outside()
    end
  end
end
