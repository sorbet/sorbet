# typed: true

def obj.foo=() = 42 rescue nil # parser-error: setter method cannot be defined in an endless method definition
