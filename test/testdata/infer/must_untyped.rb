# typed: strict

# No errors are expected
T.must(T.unsafe(nil))

# No errors are expected
T.must_because(T.unsafe(nil)) { 'reason' }
