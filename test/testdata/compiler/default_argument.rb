# frozen_string_literal: true
# typed: strict
# compiled: true

class Experiment
  extend T::Sig

  sig {returns(String)}
  attr_reader :primary_token

  sig {params(primary_token: String, analytics_id: T.nilable(String)).void}
  def initialize(primary_token, analytics_id: nil)
    @primary_token = primary_token
    @analytics_id = analytics_id
  end
end

p Experiment.new('foo_123').instance_variable_get(:@primary_token)
