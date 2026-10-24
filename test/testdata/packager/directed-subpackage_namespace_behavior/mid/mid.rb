# typed: true

module Mid
  module Inner # error: `Mid::Inner` can only define behavior if package `Mid` is marked `outer_package_with_namespace_behavior!`
    def self.helper; end
  end
end
