# frozen_string_literal: true
# typed: true
# compiled: true

class A; end

verbose = $VERBOSE
$VERBOSE = nil
Object.const_set(:A, nil)
$VERBOSE = verbose

p A.===(nil)
