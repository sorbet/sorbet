# typed: true

require 'minitest/autorun'
require 'minitest/spec'

module Searchable
  #    ^^^^^^^^^^ symbol-search: "searchable"
  class Searchable
    #   ^^^^^^^^^^ symbol-search: "searchable"

    def searchable
      #   ^^^^^^^^^^ symbol-search: "searchable"
      'searchable'
    end

    describe 'searchable' do
      #       ^^^^^^^^^^ symbol-search: "searchable"

      before do
        @searchable = searchable
      end
  
      it 'searchable' do
        # ^^^^^^^^^^ symbol-search: "searchable"
        searchable
      end

      it 'does have each letter enclosed, theoretically matching but really terrible' do
        # // ^    ^  ^        ^   ^        ^             ^       ^      ^    ^
        # // don't return for symbol-search: "searchable"
      end
    end

    describe 'does have each letter enclosed, theoretically matching but really terrible' do
      # //       ^    ^  ^        ^   ^        ^             ^       ^      ^    ^
      # // don't return for symbol-search: "searchable"
      it 'searchable' do
        # ^^^^^^^^^^ symbol-search: "searchable"
        # // but do return this one
        searchable
      end

      it 'does have each letter enclosed, theoretically matching but really terrible' do
        # // ^    ^  ^        ^   ^        ^             ^       ^      ^    ^
        # // don't return for symbol-search: "searchable"
      end
    end
  end
end