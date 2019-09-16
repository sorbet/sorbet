# typed: false
class A
  B = 1
  B = type_template # error: Redefining constant
end
