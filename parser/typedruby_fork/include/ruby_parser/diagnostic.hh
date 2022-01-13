#ifndef RUBY_PARSER_DIAGNOSTIC_HH
#define RUBY_PARSER_DIAGNOSTIC_HH

#include <cstddef>
#include <string>
#include <vector>

#include "token.hh"
#include "diagnostic_class.hh"

namespace ruby_parser {

enum class dlevel {
	NOTE    = 1,
	WARNING = 2,
	ERROR   = 3,
	FATAL   = 4,
};

class diagnostic {
public:
	struct range {
		const size_t beginPos;
		const size_t endPos;

		range(size_t beginPos, size_t endPos)
			: beginPos(beginPos)
			  , endPos(endPos)
		{}
	};

private:
	dlevel level_;
	dclass type_;
	range location_;
	std::string data_;

public:
	diagnostic(dlevel lvl, dclass type, range location, const std::string& data = "")
		: level_(lvl)
		  , type_(type)
		  , location_(location)
		  , data_(data)
	{}

	diagnostic(dlevel lvl, dclass type, const token *token, const std::string& data = "")
		: level_(lvl)
		  , type_(type)
		  , location_(token->start(), token->end())
		  , data_(data)
	{}

	dlevel level() const {
		return level_;
	}

	dclass error_class() const {
		return type_;
	}

	const std::string& data() const {
		return data_;
	}

	const range& location() const {
		return location_;
	}
};

using diagnostics_t = std::vector<diagnostic>;

}

#endif
