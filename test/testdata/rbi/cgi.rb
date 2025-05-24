# typed: true
require 'cgi'

T.assert_type!(CGI.escapeURIComponent("'Stop!' said Fred"), String)
T.assert_type!(CGI.unescapeURIComponent("%27Stop%21%27+said%20Fred"), String)
T.assert_type!(CGI.unescapeURIComponent("%27Stop%21%27+said%20Fred", "ISO-8859-1"), String)
