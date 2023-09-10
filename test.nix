rec {
  /**a.b. Docs*/
  a.b = {a ? null}: {b ? null}: a b;

  /**foo*/foonction = f: x: f x;

  /**Not a Lambda Doc but still an attrDoc*/
  c = let foo = a: a; in foo;

  attrsets = import ./attrsets.nix;


  mapAttrs = attrsets.mapAttrs;



  # builtins.getAttrDoc mapAttrs -> ""
  # builtins.getLambdaDoc mapAttrs -> "Docs"
  /**
  Doc Comment
  */
  id=x: x;

  /**Docu for number*/
  number = a.b {};

  # alias = number;

  # builtins.getDoc alias -> ""

}


# (import ./somefile)

# lib = {
#   myLibfunction = (import ./lib.nix).someLib;
#   doc = builtins.getDoc someLib;

# }

# ./lib.nix

# {
#   /*Docuemtn*/
#   someLib = x: x;

# }
