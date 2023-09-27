#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>

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
static struct Doc parseDoc(std::string sourcePrefix, const bool simple);

/*
 * A simple wrapper for building up regex. Declares the given string 's'
 * optional, non matching.
 */
static std::string optional(std::string s);

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
    // } else if (auto input = std::get_if<Pos::String>(&pos.origin)) {
    //   std::cout << "Got string: ";
    //   // auto source = *input->source;
    //   std::string source = input->source->data();

    //   std::cout << source;

    //   std::ifstream ifs(source);
    //   std::stringstream ret;
    //   size_t lineNum = 1;
    //   std::string line;
    //   // for (auto c source.le)
    //   for (int i = 0; i <= source.length(); i++) {
    //     char c = source[i];
    //   }
    //   std::cout << "L" << pos.line;
    //   std::cout << ":C" << pos.column << std::endl;
    //   while (source.split && lineNum <= pos.line) {
    //     std::cout << "line:" << line;
    //     if (lineNum < pos.line) {
    //       ret << line << "\n";
    //     } else if (lineNum == pos.line) {
    //       ret << line.substr(0, pos.column - 1);
    //     }
    //     lineNum++;
    //   }
    //   std::cout << "lastLine:" << line;
    //   std::cout << "ret:'" << ret.str() << "'";
    //   return ret.str();
    // } else if (auto input = std::get_if<Pos::Stdin>(&pos.origin)) {
    //   std::cout << "Got stdin: ";
    //   auto source = input->source->data();
    //   std::cout << source;
  } else {
    throw std::invalid_argument("pos.origin is not a path");
  }
}

struct Doc lookupDoc(const Pos &pos, const bool simple) {
  try {
    return parseDoc(readFileUpToPos(pos), simple);
  } catch (std::exception &e) {
    ignoreException();
    return emptyDoc;
  }
}
/* Try to recover a Doc by looking at the text that leads up to a term
   definition */
static struct Doc parseDoc(std::string sourcePrefix, const bool simple) {

  std::string spaces("(?:[ \\t]*)");
  std::string lineComment("(?:[\\r\\n]*[^\\r\\n]*#" + spaces + "*)");
  std::string whitespaces("(?:" + lineComment + "*[\\s]*)");
  std::string ident("(?:[a-zA-Z_][a-zA-Z0-9_'-]*)");
  std::string path("(?:(?:" + whitespaces + ident + "\\." + whitespaces + ")*" +
                   ident + ")");
  std::string assign("(?:=" + whitespaces + ")");
  std::string lParen("(?:\\(*" + whitespaces + ")");
  std::string lambda("(?:" + whitespaces + ":" + ident + lParen + ")");
  std::string doc("([ \\t]*\\/\\*[^*\\/]*\\*\\*\\/)?");

  // 1. up all whitespaces
  // 2. eat remaining parenthesis ' math.mul = ( x: ( <-| y: x * y'
  // 3. skip all eventual outer lambdas
  // 4. skip zero or one assignments to a path
  // 5. eat remaining whitespaces
  // 6. There should be the doc-comment
  std::string reverseRegex("^" + whitespaces + lParen + lambda +
                           "*(?:" + assign + path + ")?" + whitespaces + doc);
  std::string simpleRegex("^" + whitespaces + "*" + doc);

  // The comment is located at the end of the file
  // Even with $ (Anchor End) regex starts to search from the beginning of
  // the file On large and complex files this can cause infinite recursion
  // with certain patterns causing the regex to step back and never reaching
  // $ (end)
  // -> And thus never terminates. We search the comment in reverse order,
  // such that we can abort the search early This is also significantly more
  // performant. A high end solution would include a custom parser, because
  // the regex engine seems very expensive
  std::reverse(sourcePrefix.begin(), sourcePrefix.end());

  std::regex e(simpleRegex);
  if (!simple) {
    e = std::regex(reverseRegex);
  }

#define REGEX_GROUP_COMMENT 1

  std::smatch matches;
  regex_search(sourcePrefix, matches, e);

  //   std::cout << simpleRegex << std::endl;
  //   std::cout << sourcePrefix << std::endl;
  //   for (int i = 0; i < matches. ; i++) {
  //     std::cout << matches[i] << "i:" << i << std::endl;
  //   }

  std::stringstream buffer;
  if (matches.length() < REGEX_GROUP_COMMENT ||
      matches[REGEX_GROUP_COMMENT].str().empty()) {
    return emptyDoc;
  }

  std::string rawComment = matches[REGEX_GROUP_COMMENT];
  std::reverse(rawComment.begin(), rawComment.end());
  return Doc(rawComment, Doc::stripComment(rawComment));
}

std::string Doc::stripComment(std::string rawComment) {
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
