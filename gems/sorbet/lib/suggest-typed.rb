#!/usr/bin/env ruby
# frozen_string_literal: true

require_relative './step_interface'

class Sorbet; end
module Sorbet::Private; end
class Sorbet::Private::SuggestTyped
  include Sorbet::Private::StepInterface

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

  def self.output_file
    nil
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::SuggestTyped.main
end
