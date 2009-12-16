options {
  language = "Cpp";            // C++ output for antlr
  namespaceStd  = "std";       // Cosmetic option to get rid of long defines in generated code
  namespaceAntlr = "antlr";    // Cosmetic option to get rid of long defines in generated code
  namespace = "CVC4::parser";  // Wrap everything in the smtparser namespace
}
   
/**
 * AntlrCvcLexer class is a stream tokenizer (lexer) for the CVC language. 
 */
class AntlrCvcLexer extends Lexer;

options {
  exportVocab = CvcVocabulary;  // Name of the shared token vocabulary
  testLiterals = false;         // Do not check for literals by default
  defaultErrorHandler = false;  // Skip the defaul error handling, just break with exceptions
  k = 2;  
}
 
tokens {
  // Types 
  BOOLEAN       = "BOOLEAN";
  // Boolean oparators
  AND           = "AND";
  IF            = "IF";
  THEN          = "THEN";
  ELSE          = "ELSE";
  ELSEIF        = "ELSIF";
  ENDIF         = "ENDIF";
  NOT           = "NOT";
  OR            = "OR";
  TRUE          = "TRUE";
  FALSE         = "FALSE";
  XOR           = "XOR";
  IMPLIES       = "=>";
  IFF           = "<=>";
  // Commands
  ASSERT        = "ASSERT";
  QUERY         = "QUERY";
  CHECKSAT      = "CHECKSAT";
  PRINT         = "PRINT";
  EXHO          = "ECHO";
  
  PUSH          = "PUSH";
  POP           = "POP";
  POPTO         = "POPTO";
}

/**
 * Matches any letter ('a'-'z' and 'A'-'Z').
 */
protected 
ALPHA options{ paraphrase = "a letter"; } 
  : 'a'..'z' 
  | 'A'..'Z'
  ;
  
/**
 * Matches the digits (0-9)
 */
protected 
DIGIT options{ paraphrase = "a digit"; } 
  : '0'..'9'
  ;

/**
 * Matches the ':'
 */
COLON options{ paraphrase = "a colon"; } 
  : ':'
  ;

/**
 * Matches the ','
 */
COMMA options{ paraphrase = "a comma"; } 
  : ','
  ;

/**
 * Matches an identifier from the input. An identifier is a sequence of letters,
 * digits and "_", "'", "." symbols, starting with a letter. 
 */
IDENTIFIER options { paraphrase = "an identifier"; testLiterals = true; }
  : ALPHA (ALPHA | DIGIT | '_' | '\'' | '.')*
  ;
      
/**
 * Matches the left bracket ('(').
 */
LPAREN options { paraphrase = "a left parenthesis '('"; }      
  : '(';
  
/**
 * Matches the right bracket ('(').
 */
RPAREN options { paraphrase = "a right parenthesis ')'"; }  
  : ')';

/**
 * Matches and skips whitespace in the input and ignores it. 
 */
WHITESPACE options { paraphrase = "whitespace"; }
  : (' ' | '\t' | '\f')              { $setType(antlr::Token::SKIP); }
  ;

/**
 * Mathces and skips the newline symbols in the input.
 */
NEWLINE options { paraphrase = "a newline"; }  
  : ('\r' '\n' | '\r' | '\n')       { $setType(antlr::Token::SKIP); newline(); }
  ;

/**
 * Mathces the comments and ignores them
 */
COMMENT options { paraphrase = "comment"; }
  : '%' (~('\n' | '\r'))*                    { $setType(antlr::Token::SKIP); }
  ;
      
/**
 * Matches a numeral from the input (non-empty sequence of digits).
 */
NUMERAL options { paraphrase = "a numeral"; }
  : (DIGIT)+
  ;
     