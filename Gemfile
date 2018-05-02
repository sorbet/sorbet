# rubocop:disable PrisonGuard/NoRequireSideEffects

found = ['~/.', '/etc/'].any? do |file|
  begin
    path = File.expand_path(file + 'bundle-gemfile-hook')
    File.lstat(path)
  rescue
    next
  end
  instance_eval(File.read(path), path)
  break true
end
source('https://rubygems.org/') if !found

gem 'stripe-rubocop', '0.7.4'

# rubocop:enable PrisonGuard/NoRequireSideEffects
