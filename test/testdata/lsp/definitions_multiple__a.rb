# typed: true

# Documentation 2
class MyCommand
  #   ^^^^^^^^^ def: MyCommand
  #   ^^^^^^^^^ type-def: MyCommand
  #   ^^^^^^^^^ hover: Documentation 1
  #   ^^^^^^^^^ hover: Documentation 2
end

def main
  MyCommand.new
# ^^^^^^^^^ usage: MyCommand
#           ^^^ type: MyCommand
# ^^^^^^^^^ hover: Documentation 1
# ^^^^^^^^^ hover: Documentation 2
end