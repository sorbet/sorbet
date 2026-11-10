# typed: true

module Mid
  module Inner # error: `Mid::Inner` cannot define behavior, because packages are nested under `Mid::Inner`
    def self.helper; end
  end
end
