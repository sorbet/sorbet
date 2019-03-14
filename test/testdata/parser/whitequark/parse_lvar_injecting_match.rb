# typed: true
def match; end;
/(?<match>bar)/ =~ 'bar'; match # error: Method `=~` does not exist on `Regexp`
                                # this is because we run with out
