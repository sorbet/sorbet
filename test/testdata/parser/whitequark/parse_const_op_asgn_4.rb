# typed: true

def x; ::A ||= 1; end # error-with-dupes: Constant reassignment is not supported
