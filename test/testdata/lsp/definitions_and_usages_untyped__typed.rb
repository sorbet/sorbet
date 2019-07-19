# typed: true

class TypedFoo
    # ^^^^^^^^ def: TypedFoo
    # Ensures that go-to-def and find-usages works for constant refs in untyped files.
  def bar; end
    # ^^^ def: bar
    # Ensures that it ignores the method usage in the untyped file.
end

def main
  # Ensures that go-to-def/find usages works for constants defined in untyped files.
  Untyped::Bar.new.bat
# ^^^^^^^ usage: Untyped
         # ^^^ usage: Bar
  Untyped::Bar::Type
# ^^^^^^^ usage: Untyped
         # ^^^ usage: Bar
              # ^^^^ usage: Type
end
