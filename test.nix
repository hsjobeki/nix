rec {
  /**
  # Return an attribute from nested attribute sets.

  # Example

    x = { a = { b = 3; }; }
    # ["a" "b"] is equivalent to x.a.b
    # 6 is a default value to return if the path does not exist in attrset
    attrByPath ["a" "b"] 6 x
    => 3
    attrByPath ["z" "z"] 6 x
    => 6

  # Type
    attrByPath :: [String] -> Any -> AttrSet -> Any

  */
  map = x: x;

  /** Some docs*/
  add = a: b: a + b;

  addOne = add 1;

  error = rec {
    expr = builtins.unsafeGetLambdaDoc ({
        /**
        Foo docs
        */
        foo = a: b: c: a;
      }.foo "1");
  };


  /**
  This is deprecated
  */
  deprecatedMap = map;

  foo =
    builtins.unsafeGetLambdaDoc
    {
      /**
      # The id function

      * Bullet item
      * another item

      ## h2 markdown heading

      some more docs
      */
      foo = x: x;
    }
    .foo;
  /**
  Level0
    Level1
      Level2
  */
  /**
  Foonction
  */
  a.b = a: {b ? null}: a b;

  /**
  Doc2
  */
  foonction =
    /**
    A foocntion
    */
    f: (x: f + x);

  /**
  Docs
  */
  attrDoc = 1;

  lambdaDoc =
    builtins.id
    or
    /**
    Docs
    */
    (x: x);

  # Special case:
  # Lambda is directly assigned to attrName
  # It is allowed to move the lambdaDoc to the same postion as attrDoc
  mkDerivation = {
    /**
    Output Docs
    */
    outputs,
    /**
    BuildPhase Docs
    */
    buildPhase,
  }:
    derivation;

  /**
  Docs
  */
  specialLambdaDoc = z: ({
    a,
    v,
  }: (x: x));

  alias = foonction 1;

  /**
  Not a Lambda Doc but still an attrDoc
  */
  c = let foo = a: a; in foo;

  attrsets = import ./attrsets.nix;

  mapAttrs = attrsets.mapAttrs;

  # builtins.getAttrDoc mapAttrs -> ""
  # builtins.getLambdaDoc mapAttrs -> "Docs"

  /**
  Doc Comment
  */
  ${"id"} = x: x;

  /**
  Nice docs
  */
  primApp = map (x: x);
  /**
  Docu for number
  */
  partial = x: (y: (z: x));
  partiallyApplied = partial "1" "2";
  # alias = number;
  anonymous =
    builtins.id
    or
    /**
    The lib function id
    */
    (x: x);
  # builtins.getDoc alias -> ""

  /**
  Some docs
  */
  paren = x: x;

  /**
  ATTRDOC: Suppported
  LAMBDADOC: Not suppported -> NEEDED! See AST
  */
  bad1.${"foo"}.c = x: x;

  /**
  ATTRDOC: Suppported
  LAMBDADOC: Not suppported
  */
  lib = (import <nixpkgs> {}).pkgs.lib;

  # { afoo = <> foofoo = <>}

  ## CASE 1 ATTRPOS returns a postion? WOOOT!!

  setWithMappedNamed =
    lib.mapAttrs' (n: v: {
      name = "${n + "foo"}";
      /**
      Weird; afoo and bfoo attrDocs
      */
      value = v;
    })
    {
      /**
      A Lambda
      */
      a = x: x;
      /**
      Foo Lambda
      */
      b = y: y;
    };

  setWithMappedNames =
    builtins.listToAttrs
    [
      {
        name = "foo";
        /**
        FOO ALSO WEIRD DOCS
        */
        value = 123;
      }
      {
        name = "bar";
        value = 456;
      }
    ];

  # returns a cursor position at inherit| but it is impossible to abuse
  dynamicSet7 =
    builtins.zipAttrsWith
    (name: values: {inherit name values;})
    [
      {a = "x";}
      {
        a = "y";
        b = "z";
      }
    ];

  ## THIS IS NICE!!
  arguments = builtins.functionArgs (
    {
      /**
      X Docs
      */
      x,
      /**
      Y Docs
      */
      y,
      /**
      Z Docs
      */
      z,
    }:
      x + y + z
  );
  # builtins.unsafeGetAttrDocs "x" arguments -> "X Docs"

  x = builtins.derivation {
    name = "stuff";
    builder = "some name";
    system = "a";
  };

  ## CASE 2 ATTRPOS returns null

  /**
  A set containing both lambdas
  */
  dynamicSet2 = builtins.groupBy (item: builtins.head (builtins.attrNames (builtins.functionArgs item))) [({a}: a) ({b}: b) ({b}: b)];

  # getLambdaDoc bad2 -> "DOC..."

  dynamicSet3 = builtins.mapAttrs (k: v:
    /**
    Same lambda al the time
    */
    (x: x)) {
    id = "some id function";
    inv = "some inv function";
  };

  dynamicSet4 = builtins.catAttrs "a" [
    {
      /**
      A Docs
      */
      a = x: x;
    }
    {b = f: f;}
    {
      /**
      Another A Docs
      */
      a = y: y;
    }
  ];

  dynamicSet5 = builtins.partition (x: (builtins.attrNames (builtins.functionArgs x)) == []) [({a}: a) (x: x)];

  # NODE_ASSIGN
  #   NODE_ATTRPATH
  #     NODE_IDENT
  #       TOKEN_STRING "a"
  #     TOKEN_DOT "."
  #     NODE_DYNAMIC
  #       TOKEN "${"
  #       TOKEN_STRING "dynamic"
  #       TOKEN "}"
  #     TOKEN_DOT "."
  #     NODE_IDENT
  #       TOKEN_STRING "c"
  #   TOKEN_ASSIGN "="
  #   NODE_LAMBDA
  #     NODE_IDENT
  #       TOKEN_IDENT "x"
  #     TOKEN_COLON ":"
  #     NODE_IDENT
  #       TOKEN_IDENT "x"
  #   ...
  #
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

