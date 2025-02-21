# typed: false

if "flip".."flop" # 2 dots
  #^^^^^^^^^^^^^^ error: Unsupported node type `IFlipflop`
  "body 1"
end

if "flip"..."flop" # 3 dots
  #^^^^^^^^^^^^^^^ error: Unsupported node type `EFlipflop`
  "body 2"
end

"body 3" if "flip".."flop" # In a modifier
#           ^^^^^^^^^^^^^^ error: Unsupported node type `IFlipflop`

unless "flip".."flop"
  #    ^^^^^^^^^^^^^^ error: Unsupported node type `IFlipflop`
  "body 4"
end

"body 5" unless "flip".."flop" # In a modifier
#               ^^^^^^^^^^^^^^ error: Unsupported node type `IFlipflop`

# In a boolean expression
b = !("flip".."flop")
#     ^^^^^^^^^^^^^^ error: Unsupported node type `IFlipflop`
