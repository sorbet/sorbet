# typed: strict

module Chalk
  module ODM
    class Model; end
  end
end

class TokenPropsModel < Chalk::ODM::Model
  token_prop 'a'
  register_prefix 'm'
  timestamped_token_prop
  timestamped_token_prop 'bc'
  set_archive_token_prefix 'd'

  sig {void}
  def foo; end
end
