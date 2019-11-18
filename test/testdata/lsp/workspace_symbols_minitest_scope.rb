# typed: true

require 'minitest/autorun'
require 'minitest/spec'

module Outer
  class InnerTest
    describe 'foo with bar' do
      it 'baz and qux' do
        # ^^^^^^^^^^^ symbol-search: "Inner:desc foo:it baz"
        # ^^^^^^^^^^^ symbol-search: "Inner:desc bar:it qux"
      end
    end
    describe 'foo' do
      describe 'nested bar' do
        it 'baz and qux' do
          # ^^^^^^^^^^^ symbol-search: "Inner:desc foo:it baz"
          # ^^^^^^^^^^^ symbol-search: "Inner:desc bar:it qux"
        end
      end
    end
  end
end
