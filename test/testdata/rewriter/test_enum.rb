# typed: strict
class MyEnum < T::Enum
   enums do
     Foo = new
     Bar = new
     Foo = new
#    ^^^^^^^^^ error: Duplicate enum value `Foo`
   end
end
