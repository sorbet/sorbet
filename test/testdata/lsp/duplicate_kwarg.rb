# typed: true

def foo(x:, x: nil)
          # ^^^^^^ parser-error: duplicate argument name x
end

def bar(x, y:, y: nil)
             # ^^^^^^ parser-error: duplicate argument name y
end
