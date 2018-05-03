# typed: strict
module MyRoutes
  def self.registered(app)
    app.helpers Helpers
    app.get "/route/" do
      params # error: Method `params` does not exist
      my_helper
    end
  end
  module Helpers
    def my_helper
    end
  end
end
