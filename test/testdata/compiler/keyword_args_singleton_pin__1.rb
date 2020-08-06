# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './keyword_args_singleton_pin__2'

class Helper
  def self.requires_keyword_args(x:, y:)
    p x
    p y
  end
end

# See the note in the other test file. Forces the memory for the
# keywordArgsSingleton to be reclaimed. There is no longer a reference to it.
GC.start

Main.main
