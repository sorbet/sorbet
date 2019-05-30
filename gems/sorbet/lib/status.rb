# frozen_string_literal: true
# typed: true

module Sorbet::Private::Status
  def self.say(message)
    if STDOUT.isatty
      # Carriage return to reset cursor, ANSI escape code to clear current line.
      # (In case the new message is shorter than the old message.)
      print "\r\033[K#{message}"
    else
      puts message
    end
  end

  def self.done
    if STDOUT.isatty
      # Clear the require_relative output when we're done requiring.
      print "\r\033[K"
    end
  end
end
