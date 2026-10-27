# typed: true

module Consumer
  # `Provider::Flag` exists, but `Provider` is not imported. Monolithic mode
  # reports the missing import. In package-directed mode, `Provider`'s files
  # have not been loaded yet, so it currently reports an unresolved constant.
  def self.flag_enabled?
    Provider::Flag.enabled? # error: `Provider` is not imported
  end
end
