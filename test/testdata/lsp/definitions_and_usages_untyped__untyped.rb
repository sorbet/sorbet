# typed: false

module Untyped
     # ^^^^^^^ def: Untyped
  class Bar
      # ^^^ def: Bar

      Type = "Steel"
    # ^^^^ def: Type

      def bat; end
        # ^^^ def: (nothing) 1
  end
end

def main
  # go-to-def/find-refs/highlight should work on constant refs.
  TypedFoo.new.bar
# ^^^^^^^^ usage: TypedFoo
             # ^^^ def: (nothing) 2
  Untyped::Bar.new.bat
# ^^^^^^^ usage: Untyped
         # ^^^ usage: Bar
  Untyped::Bar::Type
# ^^^^^^^ usage: Untyped
         # ^^^ usage: Bar
              # ^^^^ usage: Type
end
