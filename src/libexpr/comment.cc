#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "comment.hh"
#include "nixexpr.hh"
#include "util.hh"

/* This module looks for documentation comments in the source code.

   Documentation is not retained during parsing, and it should not be,
   for performance reasons. Because of this the code has to jump
   through some hoops, to perform its task.

   Adapting the parser was not considered an option, so this code
   parses the comments from scratch, using regular expressions. These
   do not support all syntactic constructs, so in rare cases, they
   will fail and the code will report no documentation.

   One such situation is where documentation is requested for a
   partially applied function, where the outer lambda pattern
   matches an attribute set. This is not supported in the regexes
   because it potentially requires (almost?) the entire grammar.

   This module has been designed not to report the wrong
   documentation; considering that the wrong documentation is worse
   than no documentation. The regular expressions will only match
   simple, well understood syntactic structures, or not match at all.

   This approach to finding documentation does not cause extra runtime
   overhead, until used.

   This module does not support tab ('\t') characters. In some places
   they are treated as single spaces. They should be avoided.
*/
namespace nix::Comment {

struct Doc emptyDoc("", "");

/* parseDoc will try to recover a Doc by looking at the text that leads up to a
   term definition.*/
static struct Doc parseDoc(std::string sourcePrefix);

/*
 * A simple wrapper for building up regex. Declares the given string 's'
 * optional, non matching.
 */
static std::string optional(std::string s);

/* stripComment unpacks a comment, by unindenting and stripping " * " prefixes
   as applicable. The argument should include any preceding whitespace. */
static std::string stripComment(std::string rawComment);

/* Consistent unindenting. It will only remove entire columns. */
static std::string unindent(std::string s);

static std::string trimUnindent(std::string s) { return trim(unindent(s)); }

// static std::string stripPrefix(std::string prefix, std::string s) {
//   std::string::size_type index = s.find(prefix);
//   return (index == 0) ? s.erase(0, prefix.length()) : s;
// }

static std::string readFileUpToPos(const Pos &pos) {
  if (auto path = std::get_if<SourcePath>(&pos.origin)) {
    std::ifstream ifs(path->path.abs());
    std::stringstream ret;
    size_t lineNum = 1;
    std::string line;

    while (getline(ifs, line) && lineNum <= pos.line) {
      if (lineNum < pos.line) {
        ret << line << "\n";
      } else if (lineNum == pos.line) {
        ret << line.substr(0, pos.column - 1);
      }
      lineNum++;
    }

    return ret.str();
  } else {
    throw std::invalid_argument("pos.origin is not a path");
  }
}

struct Doc lookupDoc(const Pos &pos) {
  try {
    return parseDoc(readFileUpToPos(pos));
  } catch (std::exception &e) {
    ignoreException();
    return emptyDoc;
  }
}

static std::string optional(std::string s) {
  return std::string("(?:" + s + ")?");
}
static std::string optionals(std::string s) {
  return std::string("(?:" + s + ")*");
}

/* Try to recover a Doc by looking at the text that leads up to a term
   definition */
static struct Doc parseDoc(std::string sourcePrefix) {
  std::string spaces("[ \\t]*");
  std::string singleLineComment(spaces + "#[^\\r\\n]*(?:\\n|\\r\\n)");
  std::string whitespaces("([ \\t\\r\\n]|" + singleLineComment + ")*");

  std::string docCommentPrefix("\\/\\*\\*");
  std::string multilineCommentSuffix("*\\*+\\/");
  std::string docComment(docCommentPrefix + "(?:[^*]|\\*+[^*/])" +
                         multilineCommentSuffix);
  std::string ident("(?:[a-zA-Z_][a-zA-Z0-9_'-]*)" + whitespaces);
  std::string identKeep("([a-zA-Z_][a-zA-Z0-9_'-]*)" + whitespaces);
  /* lvalue for nested attrset construction, but not matching
     quoted identifiers or ${...} or comments inbetween etc */
  std::string simplePath("(?:" + whitespaces + ident + "\\.)*" + identKeep);
  std::string lambda(ident + whitespaces + ":" + whitespaces);
  /* helper to see countLambdas */
  std::string lambdas("((:?" + lambda + ")*)");
  std::string assign("=" + whitespaces);

  std::string rightParen("\\(" + whitespaces);

  // The docComment should:
  // A: be the first item from the back of the SourceString
  // B: be drecitly behind an attribute path assignment
  //
  // This is solved  by allowing an optional 'path = ' at the end.
  std::string commentUnit("(" + spaces + docComment + ")" + whitespaces +
                          optional(simplePath + assign) +
                          optionals(whitespaces + rightParen));

  std::string re(commentUnit + "$");
  std::regex e(re);

#define REGEX_GROUP_COMMENT 1

  std::smatch matches;
  regex_search(sourcePrefix, matches, e);

  std::stringstream buffer;
  if (matches.length() < REGEX_GROUP_COMMENT ||
      matches[REGEX_GROUP_COMMENT].str().empty()) {
    return emptyDoc;
  }

  std::string rawComment = matches[REGEX_GROUP_COMMENT];

  return Doc(rawComment, stripComment(rawComment));
}

static std::string stripComment(std::string rawComment) {
  rawComment.erase(rawComment.find_last_not_of("\n") + 1);

  std::string s(trimUnindent(rawComment));
  auto suffixIdx = s.find("/**");
  if (suffixIdx != std::string::npos) {
    // Preserve indentation of content in the first line.
    // Writing directly after /**, without a leading newline is a potential
    // antipattern.
    s.replace(suffixIdx, 3, "   ");
  }
  // Remove the "*/"
  if (!s.empty() && *(--s.end()) == '/')
    s.pop_back();
  if (!s.empty() && *(--s.end()) == '*')
    s.pop_back();

  s = trimUnindent(s);
  return s;
}

static std::string unindent(std::string s) {
  size_t maxIndent = 1000;
  {
    std::istringstream inStream(s);
    for (std::string line; std::getline(inStream, line);) {
      size_t firstNonWS = line.find_first_not_of(" \t\r\n");
      if (firstNonWS != std::string::npos) {
        maxIndent = std::min(firstNonWS, maxIndent);
      }
    }
  }

  std::ostringstream unindentedStream;
  {
    std::istringstream inStream(s);
    for (std::string line; std::getline(inStream, line);) {
      if (line.length() >= maxIndent) {
        unindentedStream << line.substr(maxIndent) << std::endl;
      } else {
        unindentedStream << std::endl;
      }
    }
  }
  return unindentedStream.str();
}

} // namespace nix::Comment
