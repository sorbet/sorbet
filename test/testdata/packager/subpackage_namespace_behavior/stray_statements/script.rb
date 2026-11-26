# typed: true

module StrayStatements # error: `StrayStatements` cannot define behavior, because packages are nested under `StrayStatements`
  class Main; end

  Main.new if __FILE__ == $PROGRAM_NAME # error: Unable to resolve constant `Main`
end
