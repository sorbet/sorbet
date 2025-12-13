#!/usr/bin/env ruby
# frozen_string_literal: true

require 'optparse'

module Intrinsics
  class Main
    def self.run(diff_path)
      File.open('sorbet.patch', mode: 'w') do |diff|

        # Create @sorbet//third_party/ruby/export-intrinsics.patch
        diff << "--- /dev/null\n"
        diff << "+++ b/third_party/ruby/export-intrinsics.patch\n"

        lines = []
        File.open(diff_path, mode: 'r') do |io|
          io.each_line do |line|
            lines << '+' + line
          end
        end

        diff << "@@ -0,0 +1,#{lines.size} @@\n"
        lines.each do |line|
          diff << line
        end

        # Patch @sorbet//third_party/externals.bzl
        diff << "--- a/third_party/externals.bzl\n"
        diff << "+++ b/third_party/externals.bzl\n"
        diff << "@@ -274,2 +274,3 @@\n"
        diff << "             \"@com_stripe_ruby_typer//third_party/ruby:debug_counter.h.patch\",\n"
        diff << "+            \"@com_stripe_ruby_typer//third_party/ruby:export-intrinsics.patch\",\n"
        diff << "         ],\n"
      end
    end
  end
end

if __FILE__ == $0
  diff_path = File.dirname($0) + '/export-intrinsics.patch'

  OptionParser.new do |opts|
    opts.on '-dPATH', '--diff=PATH', 'The diff file to add to sorbet ruby' do |path|
      diff_path = path
    end

    opts.on '-h', '--help', 'Show this message' do
      puts opts
      exit
    end
  end.parse!

  Intrinsics::Main.run(diff_path)
end
