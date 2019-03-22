#!/usr/bin/env ruby

# frozen_string_literal: true
# typed: true

module SorbetRBIGeneration; end

require_relative './sorbet'
require_relative './require_everything'
require_relative './gem-generator-tracepoint/tracer'
require_relative './gem-generator-tracepoint/serializer'

require 'set'

# TODO switch the Struct handling to:
#
# class Subclass < Struct(:key1, :key2)
# end
#
# generating:
#
# TemporaryStruct = Struct(:key1, :key2)
# class Subclass < TemporaryStruct
# end
#
# instead of manually defining every getter/setter

class SorbetRBIGeneration::RbiGenerator
  def self.main(output_dir = './rbi/gems/')
    trace_results = SorbetRBIGeneration::Tracer.trace do
      SorbetRBIGeneration::RequireEverything.require_everything
    end

    FileUtils.rm_r(output_dir) if Dir.exist?(output_dir)
    SorbetRBIGeneration::RbiSerializer.new(trace_results).serialize(output_dir)
  end
end

if $PROGRAM_NAME == __FILE__
  SorbetRBIGeneration::RbiGenerator.main
end
