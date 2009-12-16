options {
  language = "Cpp";            // C++ output for antlr
  namespaceStd  = "std";       // Cosmetic option to get rid of long defines in generated code
  namespaceAntlr = "antlr";    // Cosmetic option to get rid of long defines in generated code
  namespace = "CVC4::parser";  // Wrap everything in the smtparser namespace
}
   
/**
 * AntlrSmtLexer class is a stream tokenizer (lexer) for the SMT-LIB benchmark 
 * language. 
 */
class AntlrSmtLexer extends Lexer;

options {
  exportVocab = SmtVocabulary;  // Name of the shared token vocabulary
  testLiterals = false;         // Do not check for literals by default
  defaultErrorHandler = false;  // Skip the default error handling, just break with exceptions
  k = 2;  
}
 
tokens {
  // Base SMT-LIB tokens
  DISTINCT      = "distinct";
  ITE           = "ite";
  TRUE          = "true";
  FALSE         = "false";
  NOT           = "not";
  IMPLIES       = "implies";
  IF_THEN_ELSE  = "if_then_else";
  AND           = "and";
  OR            = "or";
  XOR           = "xor";
  IFF           = "iff";
  EXISTS        = "exists";
  FORALL        = "forall";
  LET           = "let";
  FLET          = "flet";
  THEORY        = "theory";
  LOGIC         = "logic";
  SAT           = "sat";
  UNSAT         = "unsat";
  UNKNOWN       = "unknown";
  BENCHMARK     = "benchmark";
  // The SMT attribute tokens
  C_LOGIC       = ":logic";
  C_ASSUMPTION  = ":assumption";
  C_FORMULA     = ":formula";
  C_STATUS      = ":status";
  C_EXTRASORTS  = ":extrasorts";
  C_EXTRAFUNS   = ":extrafuns";
  C_EXTRAPREDS  = ":extrapreds";
}

/**
 * Matches any letter ('a'-'z' and 'A'-'Z').
 */
protected 
ALPHA options{ paraphrase = "a letter"; } 
  :  'a'..'z' 
  |  'A'..'Z'
  ;
  
/**
 * Matches the digits (0-9)
 */
protected 
DIGIT options{ paraphrase = "a digit"; } 
  :   '0'..'9'
  ;

/**
 * Matches an identifier from the input. An identifier is a sequence of letters,
 * digits and "_", "'", "." symbols, starting with a letter. 
 */
IDENTIFIER options { paraphrase = "an identifier"; testLiterals = true; }
  :  ALPHA (ALPHA | DIGIT | '_' | '\'' | '.')*
  ;
  
/**
 * Matches an identifier starting with a colon. An identifier is a sequence of letters,
 * digits and "_", "'", "." symbols, starting with a colon.
 */
C_IDENTIFIER options { paraphrase = "an identifier starting with a colon"; testLiterals = true; }
  :  ':' ALPHA (ALPHA | DIGIT | '_' | '\'' | '.')*
  ;
  
/**
 * Matches the value of user-defined annotations or attributes. The only constraint imposed on a user-defined value is that it start with
 * an open brace and end with closed brace.
 */
USER_VALUE
  :   '{' 
      ( '\n' { newline(); }
      | ~('{' | '}' | '\n') 
        )* 
    '}'
  ;
  
/**
 * Matches the question mark symbol ('?'). 
 */
QUESTION_MARK options { paraphrase = "a question mark '?'"; } 
  :  '?'
  ;
  
/**
 * Matches the dollar sign ('$').
 */
DOLLAR_SIGN options { paraphrase = "a dollar sign '$'"; }   
  :  '$'
  ;
    
/**
 * Matches the left bracket ('(').
 */
LPAREN options { paraphrase = "a left parenthesis '('"; }      
  :   '(';
  
/**
 * Matches the right bracket ('(').
 */
RPAREN options { paraphrase = "a right parenthesis ')'"; }  
  :   ')';

/**
 * Matches and skips whitespace in the input. 
 */
WHITESPACE options { paraphrase = "whitespace"; }
  :  (' ' | '\t' | '\f')              { $setType(antlr::Token::SKIP); }
  ;

/**
 * Matches and skips the newline symbols in the input.
 */
NEWLINE options { paraphrase = "a newline"; }  
  :   ('\r' '\n' | '\r' | '\n')       { $setType(antlr::Token::SKIP); newline(); }
  ;
      
/**
 * Matches a numeral from the input (non-empty sequence of digits).
 */
NUMERAL options { paraphrase = "a numeral"; }
  :  (DIGIT)+
  ;
      
/**
 * Matches a double quoted string literal. No quote-escaping is supported inside.
 */
STRING_LITERAL options { paraphrase = "a string literal"; } 
  :  '\"' (~('\"'))* '\"'
  ;
  