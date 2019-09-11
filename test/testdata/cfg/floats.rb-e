# typed: true
def test_large_float
   a = 3.1415926535
   a = 0.000001
   a = -0.000001
   a = 1_0.0_0
   a = 2.2250738585072014e-307 # Float::MIN / 10
   a = 1.7976931348623157e+307 # Float::MAX / 10
   a = -2.2250738585072014e-307 # -Float::MIN / 10
   a = -1.7976931348623157e+307 # -Float::MAX / 10
   a = 2.2250738585072011e-308 # abseil rounds this to `0`
   a = 1.7976931348623159e+308 # abseil rounds this to infinity
   a = -2.2250738585072011e-308 # abseil rounds this to `0`
   a = -1.7976931348623159e+308 # abseil rounds this to infinity
end
