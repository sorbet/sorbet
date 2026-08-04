# typed: true
extend T::Sig

accented = "café"
# ^ hover: String("café")

emoji = "😀"
# ^ hover: String("😀")

sym = :café
# ^ hover: Symbol(:café)

spaced = :"hello café"
# ^ hover: Symbol(:"hello café")

sig { params(x: Integer).void }
def foo(x)
  puts(x)
end

foo("café") # error: Expected `Integer` but found `String("café")`
foo("😀") # error: Expected `Integer` but found `String("😀")`
foo(:café) # error: Expected `Integer` but found `Symbol(:café)`
foo(:"hello café") # error: Expected `Integer` but found `Symbol(:"hello café")`
