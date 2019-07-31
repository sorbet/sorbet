# typed: true

def (foo).foo; end # error: `def EXPRESSION.method` is only supported for `def self.method`
