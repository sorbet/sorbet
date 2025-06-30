# typed: true

def ("foo").foo; end # parser-error: cannot define a singleton method for a literal
