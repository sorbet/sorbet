# frozen_string_literal: true
# typed: true
# compiled: true

# When a Ruby constant is available via a global symbol, the compiler replaces
# the VM constant search with a read from that constant.
#
# That means that some const_set calls can overwrite special symbols, but the
# compiler will still behave as if they had not been overwritten.
#
# We consider this ~fine, and if we were to change something it would be to
# raise loudly at the `const_set` line, not to make the const_get resilient to
# being rewritten (specifically for these "known symbols").

verbose = $VERBOSE
$VERBOSE = nil
Object.const_set(:Integer, nil)
Object.const_set(:Module, nil)
$VERBOSE = verbose

p Integer
p Module
