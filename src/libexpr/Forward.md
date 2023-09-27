Forward

(?'doc'

[ \t]*\/\*\*
(?:[^*]|\*+[^*\/])*
\*+\/)

(?'whitespaces'
([\s]*(?'lineComment'(?'spaces'[ \t]*)#[^\r\n]*[\r\n]*)*)*) ((?'path'((?&whitespaces)(?'ident'[a-zA-Z_][a-zA-Z0-9_'-]*)\.(?&whitespaces))*(?&ident))(?'assign'(?&whitespaces)=))?

(?'lambda'(?'lParen'(?&whitespaces)\(*)*(?&ident):(?&whitespaces))*)


a: ( b: <-|

|-> :b ( :a

(?'lambda'(?&whitespaces):(?&ident)(?'lParen'\(*(?&whitespaces))*)*)*)

Backward

^(?'lineComment'[\r\n]*[^\r\n]*#(?'spaces'[ \t]*))*(?'doc'[ \t]*\/\*[^*\/]*(?=\*\*\/))


```regex
^(?'lambda'(?&whitespaces):(?&ident)(?'lParen'\(*(?&whitespaces))*)*((?'assign'=(?&whitespaces))(?'path'((?&whitespaces)(?'ident'[a-zA-Z_][a-zA-Z0-9_'-]*)\.(?&whitespaces))*(?&ident)))?(?'whitespaces'(?'lineComment'[\r\n]*[^\r\n]*#(?'spaces'[ \t]*)*)*[\s]*)*(?'doc'[ \t]*\/\*[^*\/]*\*\*\/)?
```
