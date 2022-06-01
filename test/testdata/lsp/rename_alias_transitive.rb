# typed: true

module MyModule; end

class Parent
  MyAlias = MyModule
  # ^ apply-rename: [A] newName: Foo placeholderText: MyAlias
end

class Child < Parent
  puts(MyAlias)
end
