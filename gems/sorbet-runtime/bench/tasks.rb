# frozen_string_literal: true

require 'rake/task'
require_relative 'getters'
require_relative 'setters'
require_relative 'constructor'
require_relative 'deserialize'
require_relative 'prop_definition'
require_relative 'prop_validation'
require_relative 'serialize_custom_type'
require_relative 'sigs'
require_relative 'tutils'
require_relative 'typecheck'
require_relative 'typecheck_kwargs_splat'

namespace :bench do
  task :getters do
    SorbetBenchmarks::Getters.run
  end

  task :setters do
    SorbetBenchmarks::Setters.run
  end

  task :constructor do
    SorbetBenchmarks::Constructor.run
  end

  task :deserialize do
    SorbetBenchmarks::Deserialize.run
  end

  task :prop_definition do
    SorbetBenchmarks::PropDefinition.run
  end

  task :prop_validation do
    SorbetBenchmarks::PropValidation.run
  end

  task :serialize_custom_type do
    SorbetBenchmarks::SerializeCustomType.run
  end

  task :sigs do
    SorbetBenchmarks::Sigs.run
  end

  task :typecheck do
    SorbetBenchmarks::Typecheck.run
  end

  task :typecheck_kwargs_splat do
    SorbetBenchmarks::TypecheckKwargsSplat.run
  end

  task :tutils do
    SorbetBenchmarks::TUtils.run
  end

  task all: %i[getters setters constructor deserialize prop_definition serialize_custom_type sigs tutils typecheck]
end
