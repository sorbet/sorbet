# typed: strict
# enable-experimental-rbs-comments: true

module Config
  #: (String key) -> String
  module_function def fetch(key)
    key
  end
end

T.reveal_type(Config.fetch("a")) # error: Revealed type: `String`
Config.fetch(0) # error: Expected `String` but found `Integer(0)` for argument `key`
