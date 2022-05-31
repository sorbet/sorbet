# typed: true

class A < TestThing # error: Superclasses and mixins may only use class aliases
  puts(Inner) # error: Unable to resolve constant `Inner`
end
