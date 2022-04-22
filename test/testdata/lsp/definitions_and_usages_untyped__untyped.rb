# typed: false
# We don't test that certain queries would return (nothing); we leave that to
# protocol_test_corpus.cc, since we would give those queries a "definition" of
# the `false` sigil, above.

module Untyped
     # ^^^^^^^ def: Untyped
  class Bar
      # ^^^ def: Bar

      Type = "Steel"
    # ^^^^ def: Type

      def bat; end
  end
end

def main
  # go-to-def/find-refs/highlight should work on constant refs.
  TypedFoo.new.bar
# ^^^^^^^^ usage: TypedFoo
  Untyped::Bar.new.bat
# ^^^^^^^ usage: Untyped
         # ^^^ usage: Bar
  Untyped::Bar::Type
# ^^^^^^^ usage: Untyped
         # ^^^ usage: Bar
              # ^^^^ usage: Type
end
