# typed: true
def x.method # error: `def EXPRESSION.method` is only supported for `def self.method`
end
