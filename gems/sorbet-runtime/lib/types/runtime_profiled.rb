# typed: strict
# frozen_string_literal: true

#
# From the static system, T::Utils::RuntimeProfiled is T.untyped.
#
# But from the runtime system, it's a random class (specifically, a class that
# normal programs currently don't have any instances of).
#
# Thus, T::Utils::RuntimeProfiled can be used to introduce runtime-only type
# errors. This seems like a bad idea, but it's not. It can be used to gather
# runtime type information from running code via a custom T::Configuration
# handler.
#
# This process has only ever been used at Stripe, and is likely to have rough
# edges. If you've managed to find your way here and you're curious to try it,
# please chat with us on Slack. There are no docs.
#
# See also: the --suggest-runtime-profiled flag to sorbet.
#

module T; end
module T::Utils; end
class T::Utils::RuntimeProfiled; end
