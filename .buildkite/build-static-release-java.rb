#!/usr/bin/env ruby

require 'fileutils'

# In order to accomodate other Ruby platforms, specifically Java (JRuby)
# this script will build a java pltaform gem that contains both the linux & mac binary

release_version = ENV["release_version"]
mac_platform = "universal-darwin-18"
linux_platform = "x86_64-linux"
platforms = [mac_platform, linux_platform]

FileUtils.mkdir_p 'gems/sorbet-static/libexec/'

platforms.each do |platform|

  # Unfortunately, even if a gem cannot be found `gem fetch` still returns 0
  # we rely on the unpack to catch it
  system("gem fetch sorbet-static --platform #{platform} --version #{release_version}", exception: true)

  system("gem unpack sorbet-static-#{release_version}-#{platform}.gem", exception: true)

  case platform
  when /darwin/
    FileUtils.move "sorbet-static-#{release_version}-#{platform}/libexec/sorbet", 'gems/sorbet-static/libexec/mac.sorbet'
  when /linux/
    FileUtils.move "sorbet-static-#{release_version}-#{platform}/libexec/sorbet", 'gems/sorbet-static/libexec/linux.sorbet'
  end
  
end

gemspec_filepath = 'gems/sorbet-static/sorbet-static.gemspec'
gemspec_contents = File.read(gemspec_filepath)

# modify the platform to java
gemspec_contents.gsub!("Gem::Platform::CURRENT", "\"java\"") 
gemspec_contents.gsub!("'libexec/sorbet'", "'libexec/mac.sorbet', 'libexec/linux.sorbet'") 
gemspec_contents.gsub!("0.0.0", release_version) 

# write the contents to gemspec
File.open(gemspec_filepath, "w") do |f|
  f.write(gemspec_contents)
end

Dir.chdir("gems/sorbet-static") do
  # build the java gem
  # we cd otherwise gem can't find the files in the gemspec
  system("gem build sorbet-static.gemspec", exception: true)

end

FileUtils.mkdir_p '_out_/gems'
FileUtils.move "gems/sorbet-static/sorbet-static-#{release_version}-java.gem", '_out_/gems'

