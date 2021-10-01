# typed: true
#
module MyMixin; end
class MyClass
  X = Y
  include MyMixin
  Z = T.type_alias { String }
end
