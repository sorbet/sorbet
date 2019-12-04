# typed: true

class AAA
  class BBB
  end

  XXX = 1
end

p AA # error: Unable to resolve
#   ^ completion: AAA

p AAA::B # error: Unable to resolve
#       ^ completion: BBB

p AAA::X # error: Unable to resolve
#       ^ completion: XXX
