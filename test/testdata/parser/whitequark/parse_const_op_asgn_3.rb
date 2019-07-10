# typed: true

def x; self::A ||= 1; end # error-with-dupes: Constant reassignment is not supported
