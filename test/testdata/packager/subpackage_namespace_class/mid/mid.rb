# typed: true

module Mid
  class Inner # error: `Mid::Inner` cannot be a `class`, because packages are nested under `Mid::Inner`
  end
end
