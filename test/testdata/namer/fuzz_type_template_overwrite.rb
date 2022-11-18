# typed: false
# disable-fast-path: true
class A
  B = 1
  B = type_template # error: Redefining constant `B` as a type member or type template
end
