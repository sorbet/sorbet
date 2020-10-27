# typed: true

class A
  private_class_method def A.a  # error: `def EXPRESSION.method` is only supported for `def self.method`
    5
  end
end
