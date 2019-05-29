# typed: true
# frozen_string_literal: true

#
# Hey there!
#
# T::Utils::Runtime is just a random class. Chances are[^1] it's not a class
# that your variables are instances of, which means that using it in a type
# signature will always create a type error.
#
# At first, it doesn't seem like a good thing to have a type that's sole
# purpose is to cause type errors. The trick is that within the ruby-types
# team, we only use T::Utils::RuntimeProfiled within 'generated' sigs.
#
# Unlike normal sigs, generated sigs never raise at runtime. They also log the
# actual, observed type on type error to a central location. We're using these
# observed types to refine and expand our type coverage in pay-server.
#
# What does this all mean for you?
#
# - If you were just curious, that's it! Leave the sig as is, and carry on.
# - If you wanted to replace this sig with a better, hand-authored one:
#
#   1. Remove 'generated' from the sig.
#   2. Update the sig to your liking
#
# Questions? :portal-to: => #ruby-types
#
# [^1]: Unless you happen to be calling T::Utils::RuntimeProfiled.new directly...
#

module T; end
module T::Utils; end
# Sorbet guesses this type instead of T.untyped when passed --suggest-runtime-profiled

class T::Utils::RuntimeProfiled; end
