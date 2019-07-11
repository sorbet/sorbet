# typed: true

def (foo).foo; end # error-with-dupes: `def EXPRESSION.method` is only supported for `def self.method`
