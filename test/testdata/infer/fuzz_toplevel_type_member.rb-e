# typed: true
# Adapted from fuzzed test case: https://github.com/sorbet/sorbet/issues/1087
extend T::Sig
Elem = type_member # error: `type_member` cannot be used at the top-level
sig {params(block: T.proc.params(ar0: Elem).void).void}
def yield_type_member(&block)
  yield_type_member { |y, z| }
end

