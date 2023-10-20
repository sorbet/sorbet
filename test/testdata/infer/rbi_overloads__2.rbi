# typed: true

sig { params(x: Integer).returns(Integer) }
sig { params(x: String).returns(String) }
def good_method(x)
end

sig { params(x: Integer).returns(Integer) }
sig { params(x: String).returns(String) }
def bad_method(x)
end
