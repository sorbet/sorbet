# typed: false

def m
  super # Invoke super, forwarding all the arguments, unmodified

  super() # Invoke super, explicitly with no arguments

  super(1, 2, 3)
end
