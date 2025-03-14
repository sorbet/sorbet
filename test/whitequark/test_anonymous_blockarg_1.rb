# typed: true

def foo(); bar(&); end # parser-error: no anonymous block parameter
