##################################################################################
# This file contains the keyword of a language. It doesn't clarify the semantics
# of a keyword, which will be done by RULEs, i.e., the parser will check the keyword
# while traversing the rule tables.
#
# The generated table of keywords are only used for
#  (1) Parser to check while traversing rule tables
#  (2) check the correctness of names so as not to conflict with keywords
##################################################################################

STRUCT KeyWord : ((boolean),
                  (byte),
                  (char),
                  (class),
                  (double),
                  (enum),
                  (float),
                  (int),
                  (interface),
                  (long),
                  (package),
                  (short),
                  (void),

                  (var),
                  
                  (break),
                  (case),
                  (catch),
                  (continue),
                  (default),
                  (do),
                  (else),
                  (finally),
                  (for),
                  (goto),
                  (if),
                  (return),
                  (switch),
                  (try),
                  (while),
                  
                  (abstract),
                  (const),
                  (volatile),
                  
                  (assert),
                  (new),
                  
                  (instanceof),
                  (extends),
                  (implements),
                  (import),
                  (super),
                  (synchroniz),
                  (this),
                  (throw),
                  (transient),
                  
                  (final),
                  (native),
                  (private),
                  (protected),
                  (public),
                  (static),
                  (strictfp))
