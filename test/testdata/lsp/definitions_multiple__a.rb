# typed: true

class MyCommand
  #   ^^^^^^^^^ def: MyCommand
  #   ^^^^^^^^^ type-def: MyCommand
end

def main
  MyCommand.new
# ^^^^^^^^^ usage: MyCommand
#           ^^^ type: MyCommand
end