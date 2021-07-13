# typed: true

def obj.foo=() = 42 rescue nil # error: setter method cannot be defined in an endless method definition
