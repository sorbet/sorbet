# frozen_string_literal: true
# typed: true
# compiled: true

FLAGS = {
  amp_use_new_rollout_progress: {
    flag_type: :feature,
  },
}

class Main
  def self.main
    FLAGS.transform_values {|x| x.merge(namespace: 'Opus::Amp::Flags')}
  end
end

p Main.main
