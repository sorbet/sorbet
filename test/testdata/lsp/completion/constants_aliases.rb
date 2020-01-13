# typed: true

class AAA; end
BBB = AAA
CCC = T.type_alias {AAA}

p BB # error: Unable to resolve
#   ^ completion: BBB

p CC # error: Unable to resolve
#   ^ completion: CCC
