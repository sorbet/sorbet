# frozen_string_literal: true
# typed: true
# compiled: true

# When an exception is created, it uses line number information that's stored
# in the control frame. We raise exceptions to report arity mismatches and sig
# type check failures, which means the control frames need to be set up before
# those two kinds of exceptions could be raised. We didn't always do this.

class Main
  extend T::Sig

  sig {params(x: String).void}
  def self.takes_string(x)
    # NOTE, we need to use x, as we only type check arguments that get used
    T.unsafe(x)
  end
end

begin
  Main.takes_string(T.unsafe(0))
rescue TypeError => exn
  puts 'Found `TypeError`'
  puts exn.message.match(/Expected.*/).to_s
  # We explicitly need to print the backtrace information to reliably get the
  # former crash we were seeing to happen.
  #
  # But the problem is that the backtrace information will necessarily have
  # different information in interpreted vs compiled (because there's no
  # runtime sig wrapping code). So the logic below basically says that "this
  # code doesn't crash both when interpreted and when compiled," rather than
  # "the interpreted and compiled behavior are identical"
  first_line = exn.backtrace&.fetch(0)
  if first_line =~ /call_validation_error_handler_default/
    puts 'ok'
  elsif first_line =~ /:14:in `takes_string'/
    puts 'ok'
  else
    puts 'not ok'
  end
end
