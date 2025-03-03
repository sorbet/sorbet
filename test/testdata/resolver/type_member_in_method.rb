# typed: true
def se 
  Elem = type_member
# ^^^^ parser-error: dynamic constant assignment
  #      ^^^^^^^^^^^ error: Method `type_member` does not exist on `Object`
end

