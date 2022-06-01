# typed: true

module MyModule

end

MyAlias = MyModule
# ^ apply-rename: [A] newName: Foo placeholderText: MyAlias

puts(MyAlias)
