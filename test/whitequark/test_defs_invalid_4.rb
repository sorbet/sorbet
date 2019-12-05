# typed: true

def (:"foo#{bar}").foo; end # error: cannot define a singleton method for a literal
