FileUtils.cd('/')  # change directory

FileUtils.cd('/', verbose: true)   # change directory and report it

FileUtils.cd('/') do  # change directory
  puts 'in / dir'     # do something
end                   # return to original directory
