# typed: true

# Note: We use versions below to denote the field (2) and method (1) defined by `const`.
class Computer < T::Struct
  const :make, String
# ^^^^^^^^^^^^^^^^^^^ def: make 1
  #      ^^^^ def: make 2

  def get_make
    @make
    #^^^^ usage: make 2
  end
end
Computer.new(make: "Gateway").make 
#                             ^^^^ usage: make 1
