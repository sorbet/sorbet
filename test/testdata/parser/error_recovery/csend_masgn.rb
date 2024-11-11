# typed: true

[1,2,3].each do |x|
  x&.y, = 10
 # ^^  error: &. inside multiple assignment
 # ^^  error: Used `&.` operator on `Integer`
 #   ^ error: Setter method `y=` does not exist
end
