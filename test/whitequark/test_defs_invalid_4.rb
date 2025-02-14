# typed: true

def (:"foo#{bar}").foo; end # parser-error: cannot define a singleton method for a literal
