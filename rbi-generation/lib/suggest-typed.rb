#!/usr/bin/env ruby
# frozen_string_literal: true

module SorbetRBIGeneration; end
class SorbetRBIGeneration::SuggestTyped
  def self.main
    count = 0
    while count < 100
      count += 1
      if suggest_typed
        return true
      end
    end
    puts "Adding `typed:` sigils did not converge after 100 tries."
    false
  end

  def self.suggest_typed
    IO.popen(
      ['srb', 'tc', '--suggest-typed', '--error-white-list=7022', '--typed=strict', '--silence-dev-message', '-a'],
      err: [:child, :out],
    ) do |io|
      out = io.read
      return true if out == "No errors! Great job.\n"
    end
    false
  end
end

if $PROGRAM_NAME == __FILE__
  SorbetRBIGeneration::SuggestTyped.main
end
