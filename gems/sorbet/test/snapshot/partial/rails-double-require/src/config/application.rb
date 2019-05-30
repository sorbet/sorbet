require 'rails/all'
require "active_support/dependencies"

module Blog
  class Application < Rails::Application
    config.load_defaults 5.2
    config.eager_load = false
  end
end

Rails.application.initialize!
