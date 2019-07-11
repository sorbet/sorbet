# typed: true
def x.method # error-with-dupes: `def EXPRESSION.method` is only supported for `def self.method`
end
