# @typed
def test_large_float
   a = 3.1415926535
   a = 0.000001
   a = -0.000001
   a = 2.2250738585072014e-307 # Float::MIN / 10
   a = 1.7976931348623157e+307 # Float::MAX / 10
   a = -2.2250738585072014e-307 # -Float::MIN / 10
   a = -1.7976931348623157e+307 # -Float::MAX / 10
   a = 2.2250738585072011e-308 # error: Unsupported large float literal: 2.2250738585072011e-308
   a = 1.7976931348623159e+308 # error: Unsupported large float literal
   a = -2.2250738585072011e-308 # error: Unsupported large float literal
   a = -1.7976931348623159e+308 # error: Unsupported large float literal
end
