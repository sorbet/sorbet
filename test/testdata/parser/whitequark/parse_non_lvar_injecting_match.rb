# typed: true

/#{1}(?<match>bar)/ =~ 'bar' # error: Method `=~` does not exist on `Regexp`
