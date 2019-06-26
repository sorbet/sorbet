# frozen_string_literal: true

module Sorbet::Private::UserInput
  def self.prompt_for_confirmation!(prompt)
    if ENV['SRB_YES'] != nil
      puts "SRB_YES set, continuing"
      return true
    else
      STDOUT.write "#{prompt} [Y/n] "
      if STDIN.isatty && STDOUT.isatty
        begin
          input = Kernel.gets&.strip
          if input.nil? || (input != '' && input != 'y' && input != 'Y')
            return false
          else
            return true
          end
        rescue Interrupt
          puts "\nAborting"
          Kernel.exit(1)
        end
      else
        puts "\nNot running interactivly. Set SRB_YES=1 environment variable to proceed"
        Kernel.exit(1)
      end
    end
  end
end
