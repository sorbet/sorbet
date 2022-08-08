# typed: true

md_file = Pathname.new('/usr/local/Homebrew').find do |pn|
  T.reveal_type(pn) # error: `Pathname`
end
md_file = Pathname.new('/usr/local/Homebrew').find do |pn|
  break pn if pn.to_s.end_with?('.md')
end
T.reveal_type(md_file) # error: `T.nilable(Pathname)`
