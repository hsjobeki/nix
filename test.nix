rec {
  foo = builtins.unsafeGetLambdaDoc
            {
                /**
                  # The id function

                  * Bullet item
                  * another item

                  ## h2 markdown heading

                  some more docs
                */
                foo = x: x;
            }.foo;
  /**
    Level0
      Level1
        Level2
  */
  a.b = a: {b ? null}: a b;

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
  id=((x: (x)));

  /** Nice docs*/
  primApp = map (x: x);
  /**Docu for number*/
  partial = a.b {};

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
