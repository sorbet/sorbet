#!/usr/bin/env ruby

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
    return false
  end

  def self.suggest_typed
    IO.popen(
      ['srb', 'tc', '--suggest-typed', '--error-white-list=7022', '--typed=strict', '-a'],
      err: [:child, :out],
    ) do |io|
      out = io.read
      return true if "No errors! Great job.\n" == out
    end
    return false
  end
end

if $PROGRAM_NAME == __FILE__
  SorbetRBIGeneration::SuggestTyped.main
end
