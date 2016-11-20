/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         NEW_STYPE
/* Substitute the variable and function names.  */
#define yyparse         new_parse
#define yylex           new_lex
#define yyerror         new_error
#define yydebug         new_debug
#define yynerrs         new_nerrs

#define yylval          new_lval
#define yychar          new_char

/* Copy the first part of user declarations.  */
#line 2 "AppScriptParser.y" /* yacc.c:339  */


#define YYERROR_VERBOSE 1
//#define YYDEBUG 1

#include <CAPString.h>
#include <CAPHash.h>
#include <CAPLinkedList.h>
#include <CAPStack.h>
#include <CAPThreadLock.h>

#include <capiot_common.h>

#include "AppScriptModeler.h"
	
#define DEPTH_MAX 100

#define HASH_BUCKETS 43

CAPSTRING_CONST(KEYWORD_LISTEN, "Listen");
CAPSTRING_CONST(KEYWORD_REPORT, "Report");
CAPSTRING_CONST(KEYWORD_ACTION, "Action");

typedef struct _SBehaviorData {
    cap_handle hLabelHash;
    cap_handle hExecFailLabelHash;
    cap_handle hLabelGlobalHash;
    cap_handle hExecFailLabelGlobalHash;
    cap_handle hConditionStack;
    cap_handle hLastNodeStack;
    cap_handle hGlobalLastNodeStack;
    cap_handle hGlobalConditionStack;
} SBehaviorData;

//extern int yydebug;
//int yydebug = 1;

typedef union _new_node
{
    int nValue;
    double dbValue;

    EOperator enOp;
    SExpression *pstExp;
    SCondition *pstCondition;

    /* */
    /* typedef void* cap_handle in cap_common.h */
    cap_handle hHandle;

    /* */
    char *pszStr;

    /* to pointer */
    SPeriod stPeriod;
    /* added type */
    ETimeUnit enTimeUnit;
    SThingVariable *pstThingVar;

    SExecutionStatement *pstExecStmt;

    /* */
    cap_string strString;

    SExecutionNode *pstExecNode;
    SConditionalStatement *pstCondStmt;
    STeamBehavior *pstTeamBehavior;
} new_node;

#define NEW_STYPE new_node

// final output
SScenario *g_pstScenario = NULL;
SBehaviorData *g_pstBehaviorData = NULL;


typedef struct yy_buffer_state * YY_BUFFER_STATE;

void new_error(const char *);
int new_lex(void);
extern int new_lineno;

int new_lex_destroy  (void);
YY_BUFFER_STATE new__scan_string (char * yystr );
int yyparse();

static cap_result getOperatorPrecedence(EOperator enOp, int *nPrecedence);
static CALLBACK cap_result destroyConditionParenthesis(int nOffset, void *pData, void *pUsrData);
static cap_result changeInfixToPostfix(cap_handle hConditionList, cap_handle *phPostfixConditionList);
static cap_result thingMerge(cap_string strKey, void *pData, void *pUserData);
static cap_result moveToHash(cap_string strKey, void *pData, void *pUserData);
static cap_result setLabelNextFalse(cap_string strKey, void *pData, void * pUserData);
static cap_result setNFalse(int nOffset, void *pData, void *pUserData);

// for DESTROY
static cap_result destroyThingKeywords(IN cap_string strKey, IN void *pData, IN void *pUserData);
static cap_result destroyThingVariableList(IN int nOffset, IN void *pData, IN void *pUserData);
static cap_result destroyThingVariableList(IN int nOffset, IN void *pData, IN void *pUserData);
static cap_result destroyThingVariable(IN cap_string strKey, IN void *pData, IN void *pUserData);
static cap_result destroyTeamBehavior(IN cap_string strKey, IN void *pData, IN void *pUserData);
static cap_result destroyExp(SExpression *pstExp);
static cap_result destroyExpList(IN int nOffset, IN void *pData, IN void *pUserData);
static cap_result destroyExecStmt(SExecutionStatement *pstExecStmt);
static cap_result destroyCond(SCondition *pstCond);
static cap_result destroyCondList(IN int nOffset, IN void *pData, IN void *pUserData);
static cap_result destroyCondStmt(SConditionalStatement *pstCondStmt);
static cap_result destroyExecutionNode(IN int nOffset, IN void *pData, IN void *pUserData);
static cap_result destroyStringList(IN int nOffset, IN void *pData, IN void *pUserData);


#line 186 "AppScriptParser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_NEW_APPSCRIPTPARSER_H_INCLUDED
# define YY_NEW_APPSCRIPTPARSER_H_INCLUDED
/* Debug traces.  */
#ifndef NEW_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define NEW_DEBUG 1
#  else
#   define NEW_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define NEW_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined NEW_DEBUG */
#if NEW_DEBUG
extern int new_debug;
#endif

/* Token type.  */
#ifndef NEW_TOKENTYPE
# define NEW_TOKENTYPE
  enum new_tokentype
  {
    ERROR = 258,
    INTEGER = 259,
    DOUBLE = 260,
    MASTER = 261,
    SLAVE = 262,
    COMMA = 263,
    TWOCOM = 264,
    GE = 265,
    LE = 266,
    EQ = 267,
    NE = 268,
    DOT = 269,
    NOT = 270,
    OR = 271,
    AND = 272,
    SECOND = 273,
    MINUTE = 274,
    HOUR = 275,
    DAY = 276,
    ASSIGN = 277,
    IDENTIFIER = 278,
    STRING_LITERAL = 279,
    IF = 280,
    EXECUTE = 281,
    RECEIVE = 282,
    LOOP = 283,
    WAIT_UNTIL = 284,
    ENTER = 285,
    WINDOW_ENTER = 286,
    FAIL = 287,
    PLAN_LISTEN = 288,
    PLAN_REPORT = 289,
    PLAN_ACTION = 290,
    THEN = 291,
    SEND = 292,
    FINISH = 293,
    USER = 294
  };
#endif
/* Tokens.  */
#define ERROR 258
#define INTEGER 259
#define DOUBLE 260
#define MASTER 261
#define SLAVE 262
#define COMMA 263
#define TWOCOM 264
#define GE 265
#define LE 266
#define EQ 267
#define NE 268
#define DOT 269
#define NOT 270
#define OR 271
#define AND 272
#define SECOND 273
#define MINUTE 274
#define HOUR 275
#define DAY 276
#define ASSIGN 277
#define IDENTIFIER 278
#define STRING_LITERAL 279
#define IF 280
#define EXECUTE 281
#define RECEIVE 282
#define LOOP 283
#define WAIT_UNTIL 284
#define ENTER 285
#define WINDOW_ENTER 286
#define FAIL 287
#define PLAN_LISTEN 288
#define PLAN_REPORT 289
#define PLAN_ACTION 290
#define THEN 291
#define SEND 292
#define FINISH 293
#define USER 294

/* Value type.  */
#if ! defined NEW_STYPE && ! defined NEW_STYPE_IS_DECLARED
typedef int NEW_STYPE;
# define NEW_STYPE_IS_TRIVIAL 1
# define NEW_STYPE_IS_DECLARED 1
#endif


extern NEW_STYPE new_lval;

int new_parse (void);

#endif /* !YY_NEW_APPSCRIPTPARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 323 "AppScriptParser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined NEW_STYPE_IS_TRIVIAL && NEW_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   208

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  49
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  184

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      45,    46,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
      48,     2,    47,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    43,     2,    44,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    40,     2,    41,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39
};

#if NEW_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   281,   281,   326,   330,   331,   335,   336,   340,   341,
     344,   345,   349,   350,   354,   355,   361,   376,   387,   397,
     408,   413,   421,   430,   436,   448,   458,   466,   475,   483,
     491,   504,   517,   588,   655,   659,   662,   669,   702,   709,
     701,   842,   863,   872,   901,   900,   944,   943,   973,   981,
     990,  1007,  1017,  1036,  1055,  1073,  1086,  1107,  1118,  1149,
    1161,  1170,  1179,  1181,  1183,  1185,  1189,  1194,  1198,  1206,
    1216,  1224,  1236,  1245,  1256,  1294,  1297,  1303,  1313,  1321,
    1331,  1339,  1348,  1369,  1382,  1394,  1424,  1439,  1456,  1465,
    1473,  1483,  1491,  1496,  1532,  1533,  1534,  1535,  1536,  1537,
    1538,  1539
};
#endif

#if NEW_DEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ERROR", "INTEGER", "DOUBLE", "MASTER",
  "SLAVE", "COMMA", "TWOCOM", "GE", "LE", "EQ", "NE", "DOT", "NOT", "OR",
  "AND", "SECOND", "MINUTE", "HOUR", "DAY", "ASSIGN", "IDENTIFIER",
  "STRING_LITERAL", "IF", "EXECUTE", "RECEIVE", "LOOP", "WAIT_UNTIL",
  "ENTER", "WINDOW_ENTER", "FAIL", "PLAN_LISTEN", "PLAN_REPORT",
  "PLAN_ACTION", "THEN", "SEND", "FINISH", "USER", "'{'", "'}'", "';'",
  "'['", "']'", "'('", "')'", "'>'", "'<'", "$accept", "scenario",
  "statement_start", "statement_end", "blank", "enter", "enter_item",
  "separator", "separator_item", "group_composition_list",
  "group_composition", "team_name", "thing_declaration_list",
  "thing_declaration", "thing_list", "thing_name",
  "scenario_behavior_list", "scenario_behavior", "team_behavior",
  "team_plan", "statement_list", "@1", "@2", "statement", "@3", "@4",
  "loop_condition", "expression_statement", "period_time", "time_unit",
  "target_team", "attribute_list", "attribute", "action_behavior",
  "action_input", "input", "output", "condition_list", "condition",
  "primary_expression", "biop", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     123,   125,    59,    91,    93,    40,    41,    62,    60
};
# endif

#define YYPACT_NINF -104

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-104)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -37,    -9,    70,  -104,  -104,    67,    -9,  -104,  -104,  -104,
    -104,    -9,   134,     6,  -104,   -11,  -104,  -104,    75,  -104,
      44,   134,  -104,  -104,    60,    44,  -104,   113,  -104,    78,
      64,  -104,    66,    63,    65,    86,    92,   100,   101,  -104,
      -9,   131,   109,  -104,   134,  -104,   134,  -104,  -104,  -104,
      47,    60,   143,   131,   125,    14,     9,   126,    45,    12,
       9,    45,  -104,    91,    95,   113,   134,   129,   138,    64,
     122,  -104,   127,  -104,  -104,   153,  -104,    14,   124,   167,
    -104,    14,     9,    46,  -104,    87,    11,    -1,  -104,  -104,
    -104,   169,    94,    94,   133,   170,    74,    55,   172,  -104,
    -104,   132,  -104,  -104,  -104,   131,  -104,  -104,  -104,   136,
    -104,    14,   159,   137,  -104,    14,  -104,    72,    30,     9,
       9,   148,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,
      14,   154,  -104,    45,  -104,  -104,  -104,  -104,  -104,  -104,
    -104,     9,  -104,    45,  -104,  -104,   131,    14,   139,  -104,
    -104,  -104,  -104,    74,    74,   147,  -104,   144,   174,     2,
    -104,   131,    74,    15,  -104,   145,  -104,   131,   131,   171,
     173,    45,  -104,  -104,  -104,  -104,    91,  -104,   146,  -104,
    -104,  -104,   149,  -104
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     7,     0,    11,    10,    18,     6,     8,     1,    20,
      21,     7,     0,     0,     9,     0,    14,    15,    17,    12,
       0,     0,    13,    16,     0,    19,    23,     0,    27,    24,
      25,    22,    79,     0,     0,     0,     0,     0,     0,    59,
       7,     0,     0,     2,    29,    31,    32,    37,    42,    55,
       0,     0,     0,     0,     0,    75,     0,     0,     0,    49,
       0,     0,     3,     0,     0,     0,    38,     0,     0,    26,
       0,    41,     0,    89,    90,    88,    91,     0,     0,    76,
      77,     0,     0,     0,    81,    86,    79,     0,    66,    67,
      68,     0,    89,    90,     0,    51,    52,     0,     0,     4,
      48,    38,    34,    35,    36,     0,    30,    39,    80,     0,
      28,    75,     0,     0,    72,     0,    85,     0,     0,     0,
       0,     0,    97,    98,    94,    99,   101,   100,    95,    96,
       0,     0,    57,     0,    62,    63,    64,    65,    61,    60,
      46,     0,    56,     0,     5,    33,     0,    75,     0,    93,
      92,    78,    82,    83,    84,    44,    87,     0,     0,     0,
      69,     0,    50,     0,    40,     0,    74,     0,     0,     0,
       0,     0,    54,    47,    53,    73,     0,    45,     0,    71,
      70,    43,     0,    58
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -104,  -104,    35,    17,    84,  -104,    28,    81,   -16,   179,
    -104,    -7,  -104,   175,  -104,   150,   140,  -104,  -104,  -104,
     -41,  -104,  -104,   -52,  -104,  -104,  -104,  -104,  -104,   105,
     -49,    56,    31,   151,  -103,  -104,  -104,   -54,   123,   -51,
    -104
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    41,   100,     5,     6,    17,   101,    19,    11,
      12,    13,    25,    26,    29,    30,    43,    44,    45,   105,
      46,   107,   146,    47,   168,   161,    94,    48,    95,   138,
     158,   159,   160,    49,    78,    79,    50,    83,    84,    85,
     130
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      63,    71,    22,     1,    80,    96,    97,   131,   148,    91,
     171,    22,    98,    73,    74,    20,    92,    93,    73,    74,
      42,     3,     4,   171,    81,    54,   113,    81,   117,     7,
      21,   118,    75,    76,    14,    75,    76,    75,    76,     7,
     122,   123,   124,   125,   165,   132,   126,   127,   172,    22,
      22,    88,    89,   145,    82,    67,    55,    82,    42,    77,
      80,   174,   119,   120,   151,   153,   154,    24,     7,    68,
       8,   119,   120,     9,    10,    53,   150,   128,   129,   156,
      54,     9,    10,    28,    90,    22,    51,   162,   119,   120,
     119,   120,   121,    18,   164,    15,    80,   122,   123,   124,
     125,   142,    27,   126,   127,     3,     4,    52,    56,   173,
      57,    55,   134,   135,   136,   137,   177,    16,   152,     9,
      10,     3,     4,    64,    62,    65,   176,    66,   102,   103,
     104,    58,    99,    16,   128,   129,    32,    59,    33,    34,
      35,    36,    37,     3,     4,    60,    61,    70,    72,    86,
      38,    39,   108,    40,    32,    16,    33,    34,    35,    36,
      37,   109,     3,     4,     3,     4,   110,   112,    38,    39,
     114,    40,   111,   144,    16,   115,    16,   133,   141,   140,
     143,   147,   149,   150,   155,   166,   157,    40,   170,   169,
     167,   175,   182,   181,   178,   183,   179,    23,   139,   163,
      31,    69,   180,     0,   116,   106,     0,     0,    87
};

static const yytype_int16 yycheck[] =
{
      41,    53,    18,    40,    55,    59,    60,     8,   111,    58,
       8,    27,    61,     4,     5,     9,     4,     5,     4,     5,
      27,    30,    31,     8,    15,    14,    77,    15,    82,     1,
      41,    82,    23,    24,     6,    23,    24,    23,    24,    11,
      10,    11,    12,    13,   147,    46,    16,    17,    46,    65,
      66,     6,     7,   105,    45,     8,    45,    45,    65,    45,
     111,    46,    16,    17,   115,   119,   120,    23,    40,    22,
       0,    16,    17,     6,     7,     9,    46,    47,    48,   130,
      14,     6,     7,    23,    39,   101,     8,   141,    16,    17,
      16,    17,    46,    12,   146,    11,   147,    10,    11,    12,
      13,    46,    21,    16,    17,    30,    31,    43,    45,   161,
      45,    45,    18,    19,    20,    21,   168,    42,    46,     6,
       7,    30,    31,    14,    40,    44,   167,    46,    33,    34,
      35,    45,    41,    42,    47,    48,    23,    45,    25,    26,
      27,    28,    29,    30,    31,    45,    45,     4,    23,    23,
      37,    38,    23,    40,    23,    42,    25,    26,    27,    28,
      29,    23,    30,    31,    30,    31,    44,    14,    37,    38,
      46,    40,    45,    41,    42,     8,    42,     8,     8,    46,
       8,    45,    23,    46,    36,    46,    32,    40,    14,    45,
     155,    46,    46,   176,    23,    46,    23,    18,    93,   143,
      25,    51,   171,    -1,    81,    65,    -1,    -1,    57
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    40,    50,    30,    31,    53,    54,    55,     0,     6,
       7,    58,    59,    60,    55,    53,    42,    55,    56,    57,
       9,    41,    57,    58,    23,    61,    62,    56,    23,    63,
      64,    62,    23,    25,    26,    27,    28,    29,    37,    38,
      40,    51,    60,    65,    66,    67,    69,    72,    76,    82,
      85,     8,    43,     9,    14,    45,    45,    45,    45,    45,
      45,    45,    53,    69,    14,    56,    56,     8,    22,    64,
       4,    72,    23,     4,     5,    23,    24,    45,    83,    84,
      88,    15,    45,    86,    87,    88,    23,    82,     6,     7,
      39,    79,     4,     5,    75,    77,    86,    86,    79,    41,
      52,    56,    33,    34,    35,    68,    65,    70,    23,    23,
      44,    45,    14,    88,    46,     8,    87,    86,    88,    16,
      17,    46,    10,    11,    12,    13,    16,    17,    47,    48,
      89,     8,    46,     8,    18,    19,    20,    21,    78,    78,
      46,     8,    46,     8,    41,    72,    71,    45,    83,    23,
      46,    88,    46,    86,    86,    36,    88,    32,    79,    80,
      81,    74,    86,    80,    72,    83,    46,    51,    73,    45,
      14,     8,    46,    72,    46,    46,    69,    72,    23,    23,
      81,    52,    46,    46
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    49,    50,    51,    52,    52,    53,    53,    54,    54,
      55,    55,    56,    56,    57,    57,    58,    58,    58,    59,
      60,    60,    61,    61,    62,    63,    63,    64,    64,    65,
      65,    66,    66,    67,    68,    68,    68,    69,    70,    71,
      69,    72,    72,    72,    73,    72,    74,    72,    72,    75,
      75,    75,    75,    76,    76,    76,    76,    76,    76,    76,
      77,    77,    78,    78,    78,    78,    79,    79,    79,    80,
      80,    81,    82,    82,    82,    83,    83,    84,    84,    85,
      85,    86,    86,    86,    86,    86,    87,    87,    88,    88,
      88,    88,    88,    88,    89,    89,    89,    89,    89,    89,
      89,    89
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     7,     2,     1,     2,     1,     0,     1,     2,
       1,     1,     1,     2,     1,     1,     3,     2,     0,     3,
       1,     1,     2,     1,     2,     1,     3,     1,     4,     1,
       3,     1,     1,     4,     1,     1,     1,     1,     0,     0,
       5,     3,     1,     8,     0,     7,     0,     6,     3,     0,
       3,     1,     1,     6,     6,     1,     4,     4,     9,     1,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     4,     6,     6,     0,     1,     1,     3,     1,
       3,     1,     3,     3,     3,     2,     1,     3,     1,     1,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if NEW_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !NEW_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !NEW_DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 5: /* DOUBLE  */
#line 164 "AppScriptParser.y" /* yacc.c:1257  */
      { SAFEMEMFREE(((*yyvaluep)).pszStr); }
#line 1290 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 6: /* MASTER  */
#line 165 "AppScriptParser.y" /* yacc.c:1257  */
      { SAFE_CAPSTRING_DELETE(((*yyvaluep)).strString); }
#line 1296 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 7: /* SLAVE  */
#line 166 "AppScriptParser.y" /* yacc.c:1257  */
      { SAFE_CAPSTRING_DELETE(((*yyvaluep)).strString); }
#line 1302 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 23: /* IDENTIFIER  */
#line 168 "AppScriptParser.y" /* yacc.c:1257  */
      { SAFE_CAPSTRING_DELETE(((*yyvaluep)).strString); }
#line 1308 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 24: /* STRING_LITERAL  */
#line 169 "AppScriptParser.y" /* yacc.c:1257  */
      { SAFE_CAPSTRING_DELETE(((*yyvaluep)).strString); }
#line 1314 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 39: /* USER  */
#line 167 "AppScriptParser.y" /* yacc.c:1257  */
      { SAFE_CAPSTRING_DELETE(((*yyvaluep)).strString); }
#line 1320 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 58: /* group_composition_list  */
#line 273 "AppScriptParser.y" /* yacc.c:1257  */
      {
    CAPHash_Destroy(&(((*yyvaluep)).hHandle), destroyTeamBehavior, NULL);
}
#line 1328 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 59: /* group_composition  */
#line 260 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).pstTeamBehavior != NULL)
    {
        CAPHash_Destroy(&(((*yyvaluep)).pstTeamBehavior->hThingNameHash), destroyThingVariable, NULL);

        SAFEMEMFREE(((*yyvaluep)).pstTeamBehavior->pstAction);
        SAFEMEMFREE(((*yyvaluep)).pstTeamBehavior->pstListen);
        SAFEMEMFREE(((*yyvaluep)).pstTeamBehavior->pstReport);
        SAFE_CAPSTRING_DELETE(((*yyvaluep)).pstTeamBehavior->strTeamName);
    }
    SAFEMEMFREE(((*yyvaluep)).pstTeamBehavior);
}
#line 1345 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 61: /* thing_declaration_list  */
#line 253 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).hHandle != NULL)
    {
        CAPHash_Destroy(&(((*yyvaluep)).hHandle), destroyThingVariable, NULL);
    }
}
#line 1356 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 62: /* thing_declaration  */
#line 246 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).hHandle != NULL)
    {
        CAPHash_Destroy(&(((*yyvaluep)).hHandle), destroyThingVariable, NULL);
    }
}
#line 1367 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 63: /* thing_list  */
#line 238 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).hHandle != NULL)
    {
        CAPLinkedList_Traverse(((*yyvaluep)).hHandle, destroyThingVariableList, NULL);
        CAPLinkedList_Destroy(&(((*yyvaluep)).hHandle));
    }
}
#line 1379 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 64: /* thing_name  */
#line 230 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).pstThingVar != NULL)
    {
        SAFE_CAPSTRING_DELETE(((*yyvaluep)).pstThingVar->strVarName);
        SAFEMEMFREE(((*yyvaluep)).pstThingVar);
    }
}
#line 1391 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 75: /* loop_condition  */
#line 224 "AppScriptParser.y" /* yacc.c:1257  */
      {
    destroyCondStmt(((*yyvaluep)).pstCondStmt);
    SAFEMEMFREE(((*yyvaluep)).pstCondStmt);
}
#line 1400 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 76: /* expression_statement  */
#line 198 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).pstExecNode != NULL)
    {
        destroyExecutionNode(0, ((*yyvaluep)).pstExecNode, NULL);
    }
}
#line 1411 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 80: /* attribute_list  */
#line 205 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).hHandle != NULL)
    {
        CAPLinkedList_Traverse(((*yyvaluep)).hHandle, destroyExpList, NULL);
        CAPLinkedList_Destroy(&(((*yyvaluep)).hHandle));
    }
}
#line 1423 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 82: /* action_behavior  */
#line 193 "AppScriptParser.y" /* yacc.c:1257  */
      {
    destroyExecStmt(((*yyvaluep)).pstExecStmt);
    SAFEMEMFREE(((*yyvaluep)).pstExecStmt);
}
#line 1432 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 83: /* action_input  */
#line 185 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).hHandle != NULL)
    {
        CAPLinkedList_Traverse(((*yyvaluep)).hHandle, destroyExpList, NULL);
        CAPLinkedList_Destroy(&(((*yyvaluep)).hHandle));
    }
}
#line 1444 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 85: /* output  */
#line 177 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).hHandle != NULL)
    {
        CAPLinkedList_Traverse(((*yyvaluep)).hHandle, destroyStringList, NULL);
        CAPLinkedList_Destroy(&(((*yyvaluep)).hHandle));
    }
}
#line 1456 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 86: /* condition_list  */
#line 219 "AppScriptParser.y" /* yacc.c:1257  */
      {
    CAPLinkedList_Traverse(((*yyvaluep)).hHandle, destroyCondList, NULL);
    CAPLinkedList_Destroy(&(((*yyvaluep)).hHandle));
}
#line 1465 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 87: /* condition  */
#line 213 "AppScriptParser.y" /* yacc.c:1257  */
      {
    destroyCond(((*yyvaluep)).pstCondition);
    SAFEMEMFREE(((*yyvaluep)).pstCondition);
}
#line 1474 "AppScriptParser.c" /* yacc.c:1257  */
        break;

    case 88: /* primary_expression  */
#line 172 "AppScriptParser.y" /* yacc.c:1257  */
      {
    if(((*yyvaluep)).pstExp != NULL)
        destroyExp(((*yyvaluep)).pstExp);
}
#line 1483 "AppScriptParser.c" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 282 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *tmpNode;// = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        CbFnCAPHash callback = setLabelNextFalse;
        cap_result result;

        g_pstScenario->pstGlobalExecModel->pstCurrent = g_pstScenario->pstGlobalExecModel->pstRoot;

        result = CAPHash_Traverse(g_pstBehaviorData->hExecFailLabelGlobalHash, callback, g_pstBehaviorData->hLabelGlobalHash);

        if(result != ERR_CAP_NOERROR)
        {
            // error
        }

        while(CAPStack_Pop(g_pstBehaviorData->hGlobalLastNodeStack, (void**)&tmpNode ) != ERR_CAP_NO_DATA)
        {
            if(tmpNode->pstNextTrue == NULL)
            {
                tmpNode->pstNextTrue = g_pstScenario->pstFinish;
            }
            else
            {
            }
            tmpNode = NULL;
        }

   
        while(CAPStack_Pop(g_pstBehaviorData->hGlobalConditionStack, (void**)&tmpNode) != ERR_CAP_NO_DATA)
        {
            if(tmpNode->pstNextFalse == NULL)
            {
                tmpNode->pstNextFalse = g_pstScenario->pstFinish;
            }
            else
            {
            }
            tmpNode = NULL;
        }


    }
#line 1787 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 362 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_result result;

        result = CAPHash_AddKey((yyvsp[0]).hHandle, ((yyvsp[-2]).pstTeamBehavior)->strTeamName, (yyvsp[-2]).pstTeamBehavior);
        
        if(result == ERR_CAP_DUPLICATED)
        {
            // error
        }

        (yyval).hHandle = (yyvsp[0]).hHandle;

        g_pstScenario->hGroupExecModelHash = (yyvsp[0]).hHandle;
    }
#line 1806 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 377 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hHash;

        CAPHash_Create(HASH_BUCKETS, &hHash);
        CAPHash_AddKey(hHash, ((yyvsp[-1]).pstTeamBehavior)->strTeamName, (yyvsp[-1]).pstTeamBehavior);

        (yyval).hHandle = hHash;

        g_pstScenario->hGroupExecModelHash = hHash;
    }
#line 1821 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 388 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).hHandle = NULL;

        g_pstScenario->hGroupExecModelHash = NULL;
    }
#line 1831 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 398 "AppScriptParser.y" /* yacc.c:1646  */
    {
        STeamBehavior *pstTeamBehavior = (STeamBehavior*)calloc(1, sizeof(STeamBehavior));

        pstTeamBehavior->strTeamName = (yyvsp[-2]).strString;
        pstTeamBehavior->hThingNameHash = (yyvsp[0]).hHandle;

        (yyval).pstTeamBehavior = pstTeamBehavior;
    }
#line 1844 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 409 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).strString = (yyvsp[0]).strString;
    }
#line 1852 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 414 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).strString = (yyvsp[0]).strString;
    }
#line 1860 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 422 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hHash = (yyvsp[-1]).hHandle;
        CbFnCAPHash callbackMerge = thingMerge;
        
        CAPHash_Traverse((yyvsp[0]).hHandle, callbackMerge, hHash);

        (yyval).hHandle = hHash;
    }
#line 1873 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 431 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).hHandle = (yyvsp[0]).hHandle;
    }
#line 1881 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 437 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hHash;

        CAPHash_Create(HASH_BUCKETS, &hHash);
        CAPHash_AddKey(hHash, (yyvsp[-1]).strString, (yyvsp[0]).hHandle);

        (yyval).hHandle = hHash;
        SAFE_CAPSTRING_DELETE((yyvsp[-1]).strString);
    }
#line 1895 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 449 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hList;
        CAPLinkedList_Create(&hList);

        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, (yyvsp[0]).pstThingVar);

        (yyval).hHandle = hList;
    }
#line 1908 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 459 "AppScriptParser.y" /* yacc.c:1646  */
    {
        CAPLinkedList_Add((yyvsp[-2]).hHandle, LINKED_LIST_OFFSET_LAST, 0, (yyvsp[0]).pstThingVar);

        (yyval).hHandle = (yyvsp[-2]).hHandle;
    }
#line 1918 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 467 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SThingVariable *pstThingVar = (SThingVariable*)calloc(1, sizeof(SThingVariable));

        pstThingVar->nArrayNum = 1;
        pstThingVar->strVarName = (yyvsp[0]).strString;

        (yyval).pstThingVar = pstThingVar;
    }
#line 1931 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 476 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyvsp[-3]).pstThingVar->nArrayNum = (yyvsp[-1]).nValue;
        (yyval).pstThingVar = (yyvsp[-3]).pstThingVar;
    }
#line 1940 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 484 "AppScriptParser.y" /* yacc.c:1646  */
    {
        if((yyvsp[0]).pstExecNode != NULL)
        {
            g_pstScenario->pstGlobalExecModel->pstRoot = (yyvsp[0]).pstExecNode;

        }
    }
#line 1952 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 492 "AppScriptParser.y" /* yacc.c:1646  */
    {
        if((yyvsp[-2]).pstExecNode != NULL)
        {
            g_pstScenario->pstGlobalExecModel->pstRoot = (yyvsp[-2]).pstExecNode;
        }

        (yyval).pstExecNode = (yyvsp[-2]).pstExecNode;
    }
#line 1965 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 505 "AppScriptParser.y" /* yacc.c:1646  */
    {
        CbFnCAPHash callback = setLabelNextFalse;
        cap_result result;
        result = CAPHash_Traverse(g_pstBehaviorData->hExecFailLabelHash, callback, g_pstBehaviorData->hLabelHash);
        
        // CLEAR
        CAPHash_RemoveAll(g_pstBehaviorData->hExecFailLabelHash, NULL, NULL);
        CAPHash_RemoveAll(g_pstBehaviorData->hLabelHash, NULL, NULL);
        
        (yyval).pstExecNode = NULL;
    }
#line 1981 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 518 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *tmpNode;// = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        cap_handle hStack;

        CAPStack_Create(&hStack);
        
        while(CAPStack_Pop(g_pstBehaviorData->hGlobalLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR) 
        {
            if(tmpNode->pstNextTrue == NULL)
            {
                tmpNode->pstNextTrue = (yyvsp[0]).pstExecNode;
            }
            else
            {
            }
            tmpNode = NULL;
        }

        while(CAPStack_Pop(g_pstBehaviorData->hGlobalConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR) 
        {
            if(tmpNode->pstNextFalse == NULL)
            {
                tmpNode->pstNextFalse = (yyvsp[0]).pstExecNode;
            }
            else
            {
            }
            tmpNode = NULL;
        }

        // hLabelGlobalHash, hExecFailLabelGloablHash
        CAPHash_Traverse(g_pstBehaviorData->hLabelHash, moveToHash, g_pstBehaviorData->hLabelGlobalHash);
        CAPHash_Traverse(g_pstBehaviorData->hExecFailLabelHash, moveToHash, g_pstBehaviorData->hExecFailLabelGlobalHash);
        // CLAER?


        (yyval).pstExecNode = (yyvsp[0]).pstExecNode;

        //hGlobalConditionStack, hGlobalLastNodeStack
        while(CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
        {
            CAPStack_Push(hStack, tmpNode);
            tmpNode = NULL;
        }
        while(CAPStack_Pop(hStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
        {
            CAPStack_Push(g_pstBehaviorData->hGlobalConditionStack, tmpNode);
            tmpNode = NULL;
        }


        while(CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
        {
            CAPStack_Push(hStack, tmpNode);
            printf("&&&&&&&&&&&&&&&&&&\n");
            tmpNode = NULL;
        }
        while(CAPStack_Pop(hStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
        {
            CAPStack_Push(g_pstBehaviorData->hGlobalLastNodeStack, tmpNode);
            tmpNode = NULL;
        }
        // CLAER?

        // STACK DESTROY?
        CAPStack_Destroy(&hStack, NULL, NULL);
        
    }
#line 2054 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 589 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *tmpNode;// = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        STeamBehavior *pstTeamBehavior = NULL;
        cap_result result;

        // for TEAM_BEHAVIOR => FINISH_NODE
        // LINK ALL LAST NODE TO FINISH_NODE
        while(CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) != ERR_CAP_NO_DATA) 
        {
            if(tmpNode->pstNextTrue == NULL)
            {
                tmpNode->pstNextTrue = g_pstScenario->pstFinish;
            }
            else
            {
            }
            tmpNode = NULL;
        }

        while(CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) != ERR_CAP_NO_DATA) 
        {
            if(tmpNode->pstNextFalse == NULL)
            {
                tmpNode->pstNextFalse = g_pstScenario->pstFinish;
            }
            else
            {
            }
            tmpNode = NULL;
        }

        result = CAPHash_GetDataByKey(g_pstScenario->hGroupExecModelHash, (yyvsp[-3]).strString, (void**)&pstTeamBehavior);

        if(result == ERR_CAP_NOT_FOUND)
        {
            // error
        }
        else
        {
            if(CAPString_IsEqual((yyvsp[-1]).strString, KEYWORD_LISTEN) == TRUE)
            {
                pstTeamBehavior->pstListen = (SExecutionGraph*)calloc(1, sizeof(SExecutionGraph));
                pstTeamBehavior->pstListen->pstRoot = (yyvsp[0]).pstExecNode;
                pstTeamBehavior->pstListen->pstCurrent = (yyvsp[0]).pstExecNode;
            }
            else if(CAPString_IsEqual((yyvsp[-1]).strString, KEYWORD_REPORT) == TRUE)
            {
                pstTeamBehavior->pstReport = (SExecutionGraph*)calloc(1, sizeof(SExecutionGraph));
                pstTeamBehavior->pstReport->pstRoot = (yyvsp[0]).pstExecNode;
                pstTeamBehavior->pstReport->pstCurrent = (yyvsp[0]).pstExecNode;
           
            }
            else if(CAPString_IsEqual((yyvsp[-1]).strString, KEYWORD_ACTION) == TRUE)
            {
                pstTeamBehavior->pstAction = (SExecutionGraph*)calloc(1, sizeof(SExecutionGraph));
                pstTeamBehavior->pstAction->pstRoot = (yyvsp[0]).pstExecNode;
                pstTeamBehavior->pstAction->pstCurrent = (yyvsp[0]).pstExecNode;
            }
        }

        (yyval).pstTeamBehavior = pstTeamBehavior;
        SAFE_CAPSTRING_DELETE((yyvsp[-3]).strString);
        SAFE_CAPSTRING_DELETE((yyvsp[-1]).strString);
    }
#line 2123 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 656 "AppScriptParser.y" /* yacc.c:1646  */
    {
    (yyval).strString = CAPString_New();
    CAPString_Set((yyval).strString, KEYWORD_LISTEN); }
#line 2131 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 660 "AppScriptParser.y" /* yacc.c:1646  */
    { (yyval).strString = CAPString_New();
   CAPString_Set((yyval).strString, KEYWORD_REPORT); }
#line 2138 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 663 "AppScriptParser.y" /* yacc.c:1646  */
    { (yyval).strString = CAPString_New();
    CAPString_Set((yyval).strString, KEYWORD_ACTION); }
#line 2145 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 670 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *tmpNode;
    
        if((yyvsp[0]).pstExecNode->enType == LOOP_STATEMENT)
        {
            while(CAPStack_Top(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                tmpNode->pstNextTrue = (yyvsp[0]).pstExecNode;
                CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode);
                tmpNode = NULL;
            }

            while(CAPStack_Top(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                tmpNode->pstNextFalse = (yyvsp[0]).pstExecNode;
                CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode);
                tmpNode = NULL;
            }
        }

        if((yyvsp[0]).pstExecNode->enType != IF_STATEMENT && (yyvsp[0]).pstExecNode->enType != LOOP_STATEMENT)
        {
           CAPStack_Push(g_pstBehaviorData->hLastNodeStack, (yyvsp[0]).pstExecNode);
        }
        else
        {
            CAPStack_Push(g_pstBehaviorData->hConditionStack, (yyvsp[0]).pstExecNode);
        }

        (yyval).pstExecNode = (yyvsp[0]).pstExecNode;
    }
#line 2181 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 702 "AppScriptParser.y" /* yacc.c:1646  */
    {
        int  nLen = 0;

        CAPStack_Length(g_pstBehaviorData->hLastNodeStack, &nLen);

        (yyval).nValue = nLen;
    }
#line 2193 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 709 "AppScriptParser.y" /* yacc.c:1646  */
    {
        int cLen = 0;

        CAPStack_Length(g_pstBehaviorData->hConditionStack, &cLen);

        (yyval).nValue = cLen;
    }
#line 2205 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 717 "AppScriptParser.y" /* yacc.c:1646  */
    {
        int  nLen = 0;
        int tmpLen = 0;
        cap_handle hStack = NULL;
        SExecutionNode *tmpNode;

        if((yyvsp[0]).pstExecNode->enType != IF_STATEMENT && (yyvsp[0]).pstExecNode->enType != LOOP_STATEMENT)
        {
           CAPStack_Push(g_pstBehaviorData->hLastNodeStack, (yyvsp[0]).pstExecNode);
        }

    // if statement is 'LOOP'
        if((yyvsp[0]).pstExecNode->enType == LOOP_STATEMENT)
        {
            CAPStack_Length(g_pstBehaviorData->hLastNodeStack, &nLen);
            tmpLen = nLen - (yyvsp[-2]).nValue;
            while(tmpLen--)
            {
                if(CAPStack_Top(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
                {
                    tmpNode->pstNextTrue = (yyvsp[0]).pstExecNode;
                    CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode);
                    tmpNode = NULL;
                }
            }

            CAPStack_Length(g_pstBehaviorData->hConditionStack, &nLen);
            tmpLen = nLen - (yyvsp[-1]).nValue;
            while(tmpLen--)
            {
                if(CAPStack_Top(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
                {
                    tmpNode->pstNextFalse = (yyvsp[0]).pstExecNode;
                    CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode);
                    tmpNode = NULL;
                }
            }

            while(CAPStack_Top(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                tmpNode->pstNextTrue = (yyvsp[0]).pstExecNode;
                CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode);
                tmpNode = NULL;
            }

            while(CAPStack_Top(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                tmpNode->pstNextFalse = (yyvsp[0]).pstExecNode;
                CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode);
                tmpNode = NULL;
            }

            CAPStack_Push(g_pstBehaviorData->hConditionStack, (yyvsp[0]).pstExecNode);
        }
        else
        {

        // if statement is not 'LOOP'
        // hLastNodeStack
            CAPStack_Length(g_pstBehaviorData->hLastNodeStack, &nLen);
            CAPStack_Create(&hStack);

            tmpLen = nLen - (yyvsp[-2]).nValue;

            while(tmpLen--)
            {
                if(CAPStack_Top(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
                {
                    CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode);
                    CAPStack_Push(hStack, tmpNode);
                    tmpNode = NULL;
                }
            }

            while(CAPStack_Top(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                tmpNode->pstNextTrue = (yyvsp[0]).pstExecNode;
                CAPStack_Pop(g_pstBehaviorData->hLastNodeStack, (void**)&tmpNode);
                tmpNode = NULL;
            }

            while(CAPStack_Top(hStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                CAPStack_Pop(hStack, (void**)&tmpNode);
                CAPStack_Push(g_pstBehaviorData->hLastNodeStack, tmpNode);
                tmpNode = NULL;
            }

        // hConditionStack
            CAPStack_Length(g_pstBehaviorData->hConditionStack, &nLen);

            tmpLen = nLen - (yyvsp[-1]).nValue;

            while(tmpLen--)
            {
                if(CAPStack_Top(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
                {
                    CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode);
                    CAPStack_Push(hStack, tmpNode);
                    tmpNode = NULL;
                }
            }

            while(CAPStack_Top(g_pstBehaviorData->hConditionStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                tmpNode->pstNextFalse = (yyvsp[0]).pstExecNode;
                CAPStack_Pop(g_pstBehaviorData->hConditionStack, (void**)&tmpNode);
                tmpNode = NULL;
            }

            while(CAPStack_Top(hStack, (void**)&tmpNode) == ERR_CAP_NOERROR)
            {
                    CAPStack_Pop(hStack, (void**)&tmpNode);
                    CAPStack_Push(g_pstBehaviorData->hConditionStack, tmpNode);
                    tmpNode = NULL;
            }
        }

        (yyval).pstExecNode = (yyvsp[-4]).pstExecNode;
        CAPStack_Destroy(&hStack, NULL, NULL);
    }
#line 2331 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 843 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *tmpNode;
        cap_handle hHash;
        cap_result result;

        result = CAPHash_GetDataByKey(g_pstBehaviorData->hLabelHash, (yyvsp[-2]).strString, (void**)&tmpNode);

        if(result == ERR_CAP_NOT_FOUND)
        {
            CAPHash_AddKey(g_pstBehaviorData->hLabelHash, (yyvsp[-2]).strString, (yyvsp[0]).pstExecNode);
        }
        else{
            // error
        }

        (yyval).pstExecNode = (yyvsp[0]).pstExecNode;

       SAFE_CAPSTRING_DELETE((yyvsp[-2]).strString);
    }
#line 2355 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 864 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (yyvsp[0]).pstExecNode;

        (yyval).pstExecNode = pstExecNode;

        CAPLinkedList_Add(g_pstScenario->hExecutionNodeList, LINKED_LIST_OFFSET_LAST, 0, pstExecNode);

   }
#line 2368 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 873 "AppScriptParser.y" /* yacc.c:1646  */
    {
    printf("\n\n\nSTATEMENT_LIST\n");
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement));
        cap_handle hPostfixConditionList = NULL;//= (cap_handle*)calloc(1, sizeof(cap_handle));
        cap_result result;

        pstExecNode->enType = IF_STATEMENT;
        pstExecNode->pstNextTrue = (yyvsp[-1]).pstExecNode;
        pstExecNode->nLineNo = new_lineno;
        pstExecNode->pstNextFalse = NULL;
    
        result = changeInfixToPostfix((yyvsp[-5]).hHandle, &hPostfixConditionList);

        pstCondStmt->hConditionList = hPostfixConditionList;
        pstCondStmt->stPeriod.dbTimeVal = 0;
        pstCondStmt->stPeriod.enTimeUnit = TIME_UNIT_SEC;

        pstExecNode->pstStatementData = pstCondStmt;

        (yyval).pstExecNode = pstExecNode;

        CAPStack_Push(g_pstBehaviorData->hConditionStack, pstExecNode);
        CAPLinkedList_Add(g_pstScenario->hExecutionNodeList, LINKED_LIST_OFFSET_LAST, 0, pstExecNode);
        CAPLinkedList_Traverse((yyvsp[-5]).hHandle, destroyConditionParenthesis, NULL);
        CAPLinkedList_Destroy(&((yyvsp[-5]).hHandle));
    }
#line 2400 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 901 "AppScriptParser.y" /* yacc.c:1646  */
    {
        int  nLen = 0;

        CAPStack_Length(g_pstBehaviorData->hLastNodeStack, &nLen);

        (yyval).nValue = nLen;
    }
#line 2412 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 909 "AppScriptParser.y" /* yacc.c:1646  */
    {
        printf("IF STAT\n");
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement));
        cap_handle hPostfixConditionList= NULL;// = (cap_handle*)calloc(1, sizeof(cap_handle));
        cap_result result;
        int  nLen = 0;

        pstExecNode->enType = IF_STATEMENT;
        pstExecNode->pstNextTrue = (yyvsp[0]).pstExecNode;
        pstExecNode->nLineNo = new_lineno;
        pstExecNode->pstNextFalse = NULL;
    
        result = changeInfixToPostfix((yyvsp[-4]).hHandle, &hPostfixConditionList);

        pstCondStmt->hConditionList = hPostfixConditionList;
        pstCondStmt->stPeriod.dbTimeVal = 0;
        pstCondStmt->stPeriod.enTimeUnit = TIME_UNIT_SEC;

        pstExecNode->pstStatementData = pstCondStmt;

        (yyval).pstExecNode = pstExecNode;

        if((yyvsp[-1]).nValue == nLen)
        {
            CAPStack_Push(g_pstBehaviorData->hLastNodeStack, (yyvsp[0]).pstExecNode);
        }

        CAPStack_Push(g_pstBehaviorData->hLastNodeStack, (yyvsp[-1]).pstExecNode); // because the single statement doesn't go "statement_list"
        CAPStack_Push(g_pstBehaviorData->hConditionStack, pstExecNode);
        CAPLinkedList_Add(g_pstScenario->hExecutionNodeList, LINKED_LIST_OFFSET_LAST, 0, pstExecNode);
        CAPLinkedList_Traverse((yyvsp[-4]).hHandle, destroyConditionParenthesis, NULL);
        CAPLinkedList_Destroy(&((yyvsp[-4]).hHandle));
    }
#line 2451 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 944 "AppScriptParser.y" /* yacc.c:1646  */
    {
        int  nLen = 0;

        CAPStack_Length(g_pstBehaviorData->hLastNodeStack, &nLen);

        (yyval).nValue = nLen;
    }
#line 2463 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 952 "AppScriptParser.y" /* yacc.c:1646  */
    {
        printf("PARSER LOOP\n");
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        int nLen = 0;

        pstExecNode->enType = LOOP_STATEMENT;
        pstExecNode->pstNextTrue = (yyvsp[0]).pstExecNode;
        pstExecNode->pstStatementData = (yyvsp[-3]).pstCondStmt;
        pstExecNode->nLineNo = new_lineno;
        
        (yyval).pstExecNode = pstExecNode;

        CAPStack_Length(g_pstBehaviorData->hLastNodeStack, &nLen);

        if((yyvsp[-1]).nValue == nLen)
        {
            CAPStack_Push(g_pstBehaviorData->hLastNodeStack, (yyvsp[0]).pstExecNode);
        }
        CAPLinkedList_Add(g_pstScenario->hExecutionNodeList, LINKED_LIST_OFFSET_LAST, 0, pstExecNode);

    }
#line 2489 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 974 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).pstExecNode = (yyvsp[-1]).pstExecNode;
    }
#line 2497 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 981 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement));

        pstCondStmt->hConditionList = NULL;
        pstCondStmt->stPeriod.dbTimeVal = 0;
        pstCondStmt->stPeriod.enTimeUnit = TIME_UNIT_SEC;

        (yyval).pstCondStmt = pstCondStmt;
    }
#line 2511 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 991 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement));
        cap_handle hPostfixConditionList = NULL;
        cap_result result;

        result = changeInfixToPostfix((yyvsp[0]).hHandle, &hPostfixConditionList);

        pstCondStmt->hConditionList = hPostfixConditionList;
        pstCondStmt->stPeriod.dbTimeVal = ((yyvsp[-2]).stPeriod).dbTimeVal;
        pstCondStmt->stPeriod.enTimeUnit = ((yyvsp[-2]).stPeriod).enTimeUnit;

        (yyval).pstCondStmt = pstCondStmt;
        CAPLinkedList_Traverse((yyvsp[0]).hHandle, destroyConditionParenthesis, NULL);
        CAPLinkedList_Destroy(&((yyvsp[0]).hHandle));
    }
#line 2531 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 1008 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement)); 
        pstCondStmt->hConditionList = NULL;
        pstCondStmt->stPeriod.dbTimeVal = ((yyvsp[0]).stPeriod).dbTimeVal;
        pstCondStmt->stPeriod.enTimeUnit = ((yyvsp[0]).stPeriod).enTimeUnit;

        (yyval).pstCondStmt = pstCondStmt;
    }
#line 2544 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 1018 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement));
        cap_handle hPostfixConditionList = NULL;
        cap_result result;

        result = changeInfixToPostfix((yyvsp[0]).hHandle,&hPostfixConditionList);

        pstCondStmt->hConditionList = hPostfixConditionList;
        pstCondStmt->stPeriod.dbTimeVal = 0;
        pstCondStmt->stPeriod.enTimeUnit = TIME_UNIT_SEC;

        (yyval).pstCondStmt = pstCondStmt;
        CAPLinkedList_Traverse((yyvsp[0]).hHandle, destroyConditionParenthesis, NULL);
        CAPLinkedList_Destroy(&((yyvsp[0]).hHandle));

    }
#line 2565 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 1037 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        SExecutionStatement *pstExecStmt = (SExecutionStatement*)calloc(1, sizeof(SExecutionStatement));
        SExpression * pstExp = (SExpression*)calloc(1, sizeof(SExpression));
        
        pstExp->enType = EXP_TYPE_VARIABLE;
        pstExp->strPrimaryIdentifier = (yyvsp[-3]).strString;

        CAPLinkedList_Add((yyvsp[-1]).hHandle, LINKED_LIST_OFFSET_FIRST, 0, pstExp);
        pstExecStmt->hInputList = (yyvsp[-1]).hHandle;

        pstExecNode->enType = SEND_STATEMENT;
        pstExecNode->pstStatementData = pstExecStmt;
        pstExecNode->nLineNo = new_lineno;

        (yyval).pstExecNode = pstExecNode;
    }
#line 2587 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 1056 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        SExecutionStatement *pstExecStmt = (SExecutionStatement*)calloc(1, sizeof(SExecutionStatement));
        SExpression * pstExp = (SExpression*)calloc(1, sizeof(SExpression));
        
        pstExp->enType = EXP_TYPE_VARIABLE;
        pstExp->strPrimaryIdentifier = (yyvsp[-3]).strString;
        CAPLinkedList_Add((yyvsp[-1]).hHandle, LINKED_LIST_OFFSET_FIRST, 0, pstExp);
        pstExecStmt->hInputList = (yyvsp[-1]).hHandle;

        pstExecNode->enType = RECEIVE_STATEMENT;
        pstExecNode->pstStatementData = pstExecStmt;
        pstExecNode->nLineNo = new_lineno;

        (yyval).pstExecNode = pstExecNode;
    }
#line 2608 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 1074 "AppScriptParser.y" /* yacc.c:1646  */
    {
        printf("PARSER ACTION_BEHAVIOR\n");
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));

        pstExecNode->pstStatementData = (yyvsp[0]).pstExecStmt;
        pstExecNode->enType = ACTION_STATEMENT;
        pstExecNode->nLineNo = new_lineno;

        (yyval).pstExecNode = pstExecNode;
    }
#line 2623 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 1087 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)calloc(1, sizeof(SConditionalStatement));
        cap_handle hPostfixConditionList = NULL;//= (cap_handle*)calloc(1, sizeof(cap_handle));
        cap_result result;

        // cap_result 처리?
        // INFIX TO POSTFIX
        result = changeInfixToPostfix((yyvsp[-1]).hHandle, &hPostfixConditionList);

        pstCondStmt->hConditionList = hPostfixConditionList;
        pstExecNode->pstStatementData = pstCondStmt;
        pstExecNode->enType = WAIT_UNTIL_STATEMENT;
        pstExecNode->nLineNo = new_lineno;

        (yyval).pstExecNode = pstExecNode;
        CAPLinkedList_Traverse((yyvsp[-1]).hHandle, destroyConditionParenthesis, NULL);
        CAPLinkedList_Destroy(&((yyvsp[-1]).hHandle));
    }
#line 2647 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 1108 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        
        pstExecNode->pstStatementData = (yyvsp[-1]).pstExecStmt;
        pstExecNode->enType = ACTION_STATEMENT;
        pstExecNode->nLineNo = new_lineno;

        (yyval).pstExecNode = pstExecNode;
    }
#line 2661 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 1119 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        cap_handle tmpList;
        
        cap_result result;
        cap_handle hList;
        
        pstExecNode->pstStatementData = (yyvsp[-6]).pstExecStmt;
        pstExecNode->enType = ACTION_STATEMENT;
        pstExecNode->nLineNo = new_lineno;

        /* case of FAIL */
        result = CAPHash_GetDataByKey(g_pstBehaviorData->hExecFailLabelHash, (yyvsp[-2]).strString, &tmpList);

        if(result == ERR_CAP_NOT_FOUND)
        {
            CAPLinkedList_Create(&hList);
            CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, pstExecNode);
            CAPHash_AddKey(g_pstBehaviorData->hExecFailLabelHash, (yyvsp[-2]).strString, hList);
        }
        else
        {
            CAPLinkedList_Add(tmpList, LINKED_LIST_OFFSET_LAST, 0, pstExecNode);
        }

        (yyval).pstExecNode = pstExecNode;

        SAFE_CAPSTRING_DELETE((yyvsp[-2]).strString);
     }
#line 2695 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 1150 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionNode *pstExecNode = (SExecutionNode*)calloc(1, sizeof(SExecutionNode));
        
        pstExecNode->enType = FINISH_STATEMENT;
        pstExecNode->nLineNo = new_lineno;

        (yyval).pstExecNode = pstExecNode;
        printf("FINISH\n");
    }
#line 2709 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 1162 "AppScriptParser.y" /* yacc.c:1646  */
    {
    	char *pszEndPtr = NULL;
        (yyval).stPeriod.dbTimeVal = strtod((yyvsp[-1]).pszStr, &pszEndPtr); 
        (yyval).stPeriod.enTimeUnit = (yyvsp[0]).enTimeUnit;
        
        free((yyvsp[-1]).pszStr);
    }
#line 2721 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 1171 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).stPeriod.dbTimeVal = (yyvsp[-1]).nValue; 
        (yyval).stPeriod.enTimeUnit = (yyvsp[0]).enTimeUnit;
    }
#line 2730 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 1180 "AppScriptParser.y" /* yacc.c:1646  */
    { (yyval).enTimeUnit = TIME_UNIT_SEC; }
#line 2736 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 1182 "AppScriptParser.y" /* yacc.c:1646  */
    { (yyval).enTimeUnit = TIME_UNIT_MINUTE; }
#line 2742 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 1184 "AppScriptParser.y" /* yacc.c:1646  */
    { (yyval).enTimeUnit = TIME_UNIT_HOUR; }
#line 2748 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 1186 "AppScriptParser.y" /* yacc.c:1646  */
    { (yyval).enTimeUnit = TIME_UNIT_DAY; }
#line 2754 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 1190 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).strString = (yyvsp[0]).strString;
    }
#line 2762 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 1195 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).strString = (yyvsp[0]).strString;
    }
#line 2770 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 1199 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).strString = (yyvsp[0]).strString;
    }
#line 2778 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 1207 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hList;
        CAPLinkedList_Create(&hList);

        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, (yyvsp[0]).pstExp);
		 //printf(">>>>>>>>>>>>>%08llx %08llx\n", $1.pstExp, $1.pstExp->strPrimaryIdentifier);   
        (yyval).hHandle = hList;
    }
#line 2791 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 1217 "AppScriptParser.y" /* yacc.c:1646  */
    {
        CAPLinkedList_Add((yyvsp[-2]).hHandle, LINKED_LIST_OFFSET_LAST, 0, (yyvsp[0]).pstExp);

        (yyval).hHandle = (yyvsp[-2]).hHandle;
    }
#line 2801 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 1225 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));

        pstExp->enType = EXP_TYPE_MEMBER_VARIABLE;
        pstExp->strPrimaryIdentifier = (yyvsp[-2]).strString;
        pstExp->strSubIdentifier = (yyvsp[0]).strString;

        (yyval).pstExp = pstExp;
    }
#line 2815 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 1237 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionStatement *pstExecStmt = (SExecutionStatement*)calloc(1, sizeof(SExecutionStatement)); 

        pstExecStmt->hInputList = (yyvsp[-1]).hHandle;
        pstExecStmt->strPrimaryIdentifier = (yyvsp[-3]).strString;

        (yyval).pstExecStmt = pstExecStmt;
    }
#line 2828 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 1246 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionStatement *pstExecStmt = (SExecutionStatement*)calloc(1, sizeof(SExecutionStatement));
        cap_result result;

        pstExecStmt->hInputList = (yyvsp[-1]).hHandle;
        pstExecStmt->hOutputList = (yyvsp[-5]).hHandle;
        pstExecStmt->strPrimaryIdentifier = (yyvsp[-3]).strString;

        (yyval).pstExecStmt = pstExecStmt;
    }
#line 2843 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 1257 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExecutionStatement *pstExecStmt = (SExecutionStatement*)calloc(1, sizeof(SExecutionStatement));
        SThingKeywords *pstThingKeyword = NULL; 
        cap_result result;

        pstExecStmt->strPrimaryIdentifier = (yyvsp[-5]).strString;
        pstExecStmt->strSubIdentifier = (yyvsp[-3]).strString;

        pstExecStmt->hInputList =(yyvsp[-1]).hHandle;

        (yyval).pstExecStmt = pstExecStmt;

        // register Thing's values OR functions
        result = CAPHash_GetDataByKey(g_pstScenario->hThingHash, pstExecStmt->strPrimaryIdentifier, (void**)&pstThingKeyword);

        if( result == ERR_CAP_NOT_FOUND )
        {   
            pstThingKeyword = (SThingKeywords*)calloc(1, sizeof(SThingKeywords));

            CAPHash_Create(HASH_BUCKETS, &(pstThingKeyword->hValueHash));
            CAPHash_Create(HASH_BUCKETS, &(pstThingKeyword->hFunctionHash));
            pstThingKeyword->strThingRealId = CAPString_New();
                
            CAPHash_AddKey(g_pstScenario->hThingHash, pstExecStmt->strPrimaryIdentifier, pstThingKeyword);
        }   

        result = CAPHash_AddKey(pstThingKeyword->hFunctionHash, pstExecStmt->strSubIdentifier, NULL);

        if( result == ERR_CAP_DUPLICATED )
        {   
            // ignore the error
        }   

    }
#line 2882 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 1294 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).hHandle = NULL;
    }
#line 2890 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 1298 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).hHandle = (yyvsp[0]).hHandle;
    }
#line 2898 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 1304 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hList;
        CAPLinkedList_Create(&hList);
        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, (yyvsp[0]).pstExp);
        //printf(">>>>>>>>>>>>>%08llx %08llx\n", $1.pstExp, $1.pstExp->strPrimaryIdentifier);

        (yyval).hHandle = hList;
    }
#line 2911 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 1314 "AppScriptParser.y" /* yacc.c:1646  */
    {
        CAPLinkedList_Add((yyvsp[-2]).hHandle, LINKED_LIST_OFFSET_LAST, 0, (yyvsp[0]).pstExp);

        (yyval).hHandle = (yyvsp[-2]).hHandle;
    }
#line 2921 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 1322 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hList;
        CAPLinkedList_Create(&hList);

        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, (yyvsp[0]).strString);

        (yyval).hHandle = hList;
    }
#line 2934 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 1332 "AppScriptParser.y" /* yacc.c:1646  */
    {
        CAPLinkedList_Add((yyvsp[-2]).hHandle, LINKED_LIST_OFFSET_LAST, 0, (yyvsp[0]).strString);

        (yyval).hHandle = (yyvsp[-2]).hHandle;
    }
#line 2944 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 1340 "AppScriptParser.y" /* yacc.c:1646  */
    {
        printf("condition2\n");
        cap_handle hList;
        CAPLinkedList_Create(&hList);
        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, (yyvsp[0]).hHandle);

        (yyval).hHandle = hList;
    }
#line 2957 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 1349 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hList;
        CAPLinkedList_Create(&hList);

        SCondition *pstConditionLeft = (SCondition*)calloc(1, sizeof(SCondition));
        pstConditionLeft->enOperator = OPERATOR_LEFT_PARENTHESIS;
        pstConditionLeft->bConditionTrue = FALSE;
        pstConditionLeft->bInit = TRUE;
        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_FIRST, 0, pstConditionLeft);

        CAPLinkedList_Attach(hList, &((yyvsp[-1]).hHandle));

        SCondition *pstConditionRight = (SCondition*)calloc(1, sizeof(SCondition));
        pstConditionRight->enOperator = OPERATOR_RIGHT_PARENTHESIS;
        pstConditionRight->bConditionTrue = FALSE;
        pstConditionRight->bInit = TRUE;
        CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_LAST, 0, pstConditionRight);

        (yyval).hHandle = hList;
    }
#line 2982 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 1370 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SCondition *pstCondition = (SCondition*)calloc(1, sizeof(SCondition));
        pstCondition->enOperator = OPERATOR_OR;
        pstCondition->bConditionTrue = FALSE;
        pstCondition->bInit = TRUE;

        CAPLinkedList_Add((yyvsp[-2]).hHandle, LINKED_LIST_OFFSET_LAST, 0, pstCondition);
        CAPLinkedList_Attach((yyvsp[-2]).hHandle, &((yyvsp[0]).hHandle));

        (yyval).hHandle = (yyvsp[-2]).hHandle;
    }
#line 2998 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 1383 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SCondition *pstCondition = (SCondition*)calloc(1, sizeof(SCondition));
        pstCondition->enOperator = OPERATOR_AND;
        pstCondition->bConditionTrue = FALSE;
        pstCondition->bInit = TRUE;

        CAPLinkedList_Add((yyvsp[-2]).hHandle, LINKED_LIST_OFFSET_LAST, 0, pstCondition);
        CAPLinkedList_Attach((yyvsp[-2]).hHandle, &((yyvsp[0]).hHandle));

        (yyval).hHandle = (yyvsp[-2]).hHandle;
    }
#line 3014 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 1395 "AppScriptParser.y" /* yacc.c:1646  */
    {
        cap_handle hList;
        CAPLinkedList_Create(&hList);

        switch((yyvsp[0]).pstCondition->enOperator)
        {
            case OPERATOR_NOT_EQUAL: (yyvsp[0]).pstCondition->enOperator = OPERATOR_EQUAL;
                                     break;
            case OPERATOR_EQUAL: (yyvsp[0]).pstCondition->enOperator = OPERATOR_NOT_EQUAL;
                                     break;
            case OPERATOR_GREATER: (yyvsp[0]).pstCondition->enOperator = OPERATOR_LESS_EQUAL;
                                     break;
            case OPERATOR_LESS: (yyvsp[0]).pstCondition->enOperator = OPERATOR_GREATER_EQUAL;
                                     break;
            case OPERATOR_GREATER_EQUAL: (yyvsp[0]).pstCondition->enOperator = OPERATOR_LESS;
                                     break;
            case OPERATOR_LESS_EQUAL: (yyvsp[0]).pstCondition->enOperator = OPERATOR_GREATER;
                                     break;
            default: /* error? */
                                     break;
        }

        CAPLinkedList_Add (hList, LINKED_LIST_OFFSET_LAST, 0, (yyvsp[0]).pstCondition);
        (yyval).hHandle = hList;

    }
#line 3045 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 1425 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SCondition *pstCondition = (SCondition*)calloc(1, sizeof(SCondition));
        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));

        pstCondition->pstLeftOperand = (yyvsp[0]).pstExp;
        pstCondition->enOperator = OPERATOR_NOT_EQUAL;
        pstCondition->pstRightOperand = pstExp;
        pstCondition->pstRightOperand->enType = EXP_TYPE_INTEGER;
        pstCondition->pstRightOperand->dbValue = 0;
        pstCondition->bConditionTrue = FALSE;
        pstCondition->bInit = TRUE;

        (yyval).pstCondition = pstCondition;
    }
#line 3064 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 1440 "AppScriptParser.y" /* yacc.c:1646  */
    {
        printf("PRIMARY BIOP PRIMARY\n");
        SCondition *pstCondition = (SCondition*)calloc(1, sizeof(SCondition));

        pstCondition->pstLeftOperand = (yyvsp[-2]).pstExp;
        pstCondition->enOperator = (yyvsp[-1]).enOp;
        pstCondition->pstRightOperand = (yyvsp[0]).pstExp;
        pstCondition->bConditionTrue = FALSE;
        pstCondition->bInit = TRUE;

        (yyval).pstCondition = pstCondition;
    
    }
#line 3082 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1457 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));

    // var name: tmp?
        pstExp->strPrimaryIdentifier = (yyvsp[0]).strString;
        pstExp->enType = EXP_TYPE_VARIABLE;
        (yyval).pstExp = pstExp;
    }
#line 3095 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1466 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));

        pstExp->dbValue = (yyvsp[0]).nValue;
        pstExp->enType = EXP_TYPE_INTEGER;
        (yyval).pstExp = pstExp; 
    }
#line 3107 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1474 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));
        char *pszEndPtr = NULL;

        pstExp->dbValue = strtod((yyvsp[0]).pszStr, &pszEndPtr); 
        pstExp->enType = EXP_TYPE_DOUBLE;
        (yyval).pstExp = pstExp;
        SAFEMEMFREE((yyvsp[0]).pszStr);
    }
#line 3121 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1484 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));
        pstExp->strStringValue = (yyvsp[0]).strString;

        pstExp->enType = EXP_TYPE_STRING;
        (yyval).pstExp = pstExp;
    }
#line 3133 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1492 "AppScriptParser.y" /* yacc.c:1646  */
    {
        (yyval).pstExp = (yyvsp[-1]).pstExp;
    }
#line 3141 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1497 "AppScriptParser.y" /* yacc.c:1646  */
    {
        SThingKeywords *pstThingKeyword = NULL;
        cap_result result;

        SExpression *pstExp = (SExpression*)calloc(1, sizeof(SExpression));

        pstExp->strPrimaryIdentifier = (yyvsp[-2]).strString;
        pstExp->strSubIdentifier = (yyvsp[0]).strString;

        pstExp->enType = EXP_TYPE_MEMBER_VARIABLE;
        (yyval).pstExp = pstExp;

        // register Thing's values OR functions
        result = CAPHash_GetDataByKey(g_pstScenario->hThingHash, pstExp->strPrimaryIdentifier, (void**)&pstThingKeyword);

        if( result == ERR_CAP_NOT_FOUND )
        {
            pstThingKeyword = (SThingKeywords*)calloc(1, sizeof(SThingKeywords));

            CAPHash_Create(HASH_BUCKETS, &(pstThingKeyword->hValueHash));
            CAPHash_Create(HASH_BUCKETS, &(pstThingKeyword->hFunctionHash));
            pstThingKeyword->strThingRealId = CAPString_New();
            
            CAPHash_AddKey(g_pstScenario->hThingHash, pstExp->strPrimaryIdentifier, pstThingKeyword);
        }


        result = CAPHash_AddKey(pstThingKeyword->hValueHash, pstExp->strSubIdentifier, NULL);

        if( result == ERR_CAP_DUPLICATED )
        {
            // ignore the error
        }
    }
#line 3180 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1532 "AppScriptParser.y" /* yacc.c:1646  */
    { printf("==\n"); (yyval).enOp = OPERATOR_EQUAL; }
#line 3186 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1533 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_GREATER; }
#line 3192 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 1534 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_LESS; }
#line 3198 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1535 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_GREATER_EQUAL; }
#line 3204 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1536 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_LESS_EQUAL; }
#line 3210 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1537 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_NOT_EQUAL;}
#line 3216 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1538 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_AND; }
#line 3222 "AppScriptParser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1539 "AppScriptParser.y" /* yacc.c:1646  */
    {(yyval).enOp = OPERATOR_OR; }
#line 3228 "AppScriptParser.c" /* yacc.c:1646  */
    break;


#line 3232 "AppScriptParser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1541 "AppScriptParser.y" /* yacc.c:1906  */




// for MERGE in ThingDeclarationList
static cap_result thingMerge(cap_string strKey, void *pData, void *pUserData)
{
    /*
        hList: ThingList,
        hHash: ThingDeclarationList
    */
    cap_handle hList = pData;
    cap_handle hHash = pUserData;
    cap_handle tmpHash;
    cap_result result;

    result = CAPHash_AddKey(hHash, strKey, hList);

    if(result == ERR_CAP_DUPLICATED)
    {
        CAPHash_GetDataByKey(hHash, strKey, &tmpHash);

        // ThingName List?
        CAPLinkedList_Attach(tmpHash, &hList);
    }

    return ERR_CAP_NOERROR;
}

// for MOVE HASH in ScenarioBehavior
static cap_result moveToHash(cap_string strKey, void *pData, void *pUserData)
{
    SExecutionNode *pstExecNode = (SExecutionNode*)pData;
    cap_handle pstGlobalHash = pUserData;
    cap_result result;

    result = CAPHash_AddKey(pstGlobalHash, strKey, pstExecNode);

    if(result == ERR_CAP_DUPLICATED)
    {
        // error
    }

    return ERR_CAP_NOERROR;
}

static CALLBACK cap_result destroyConditionParenthesis(int nOffset, void *pData, void *pUsrData) {
    cap_result result = ERR_CAP_UNKNOWN;
    SCondition *pstData = (SCondition *)pData;

    // Remove only the condition is left or right  parenthesis operator
    if(pstData->enOperator == OPERATOR_LEFT_PARENTHESIS || pstData->enOperator == OPERATOR_RIGHT_PARENTHESIS)
    {
        SAFEMEMFREE(pstData);
    }

    result = ERR_CAP_NOERROR;
    return result;
}

static cap_result setLabelNextFalse(cap_string strKey, void *pData, void * pUserData)
{
    cap_handle hFGList = pData;
    cap_handle hGLHash = pUserData;
    SExecutionNode *pstExecNode;
    cap_result result;
    CbFnCAPLinkedList callback = setNFalse;

    result = CAPHash_GetDataByKey(hGLHash, strKey, (void**)&pstExecNode);

    if(result == ERR_CAP_NOT_FOUND)
    {
        // error
    }
    else
    {
        CAPLinkedList_Traverse(hFGList, callback, pstExecNode);
    }
    
    return ERR_CAP_NOERROR;
}

static cap_result setNFalse(int nOffset, void *pData, void *pUserData)
{
    SExecutionNode *pstExecNode = (SExecutionNode*)pData;
    SExecutionNode *tmpExecNode = (SExecutionNode*)pUserData;

    pstExecNode->pstNextFalse = tmpExecNode;
    return ERR_CAP_NOERROR;
}

static cap_result getOperatorPrecedence(EOperator enOp, int *nPrecedence){
    cap_result result = ERR_CAP_UNKNOWN;
    
    switch(enOp)
    {
        case OPERATOR_NONE: 
            *nPrecedence = 0; 
            break;
        case OPERATOR_LEFT_PARENTHESIS: 
            *nPrecedence = 1; 
            break;
        case OPERATOR_OR:
            *nPrecedence = 2; 
            break;
        case OPERATOR_AND:
            *nPrecedence = 3; 
            break;
        default:
            ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

// TODO

static cap_result changeInfixToPostfix(cap_handle hConditionList, cap_handle *phPostfixConditionList)
{
    void* pData;
    int nConditionSize;
    SCondition* pstTopCondition = NULL;
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hStack = NULL;
    cap_handle hPostfixConditionList = NULL;
    SCondition *pstStackBottomCondition = NULL;
    int Loop = 0;

    IFVARERRASSIGNGOTO(hConditionList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPLinkedList_Create(&hPostfixConditionList);
    ERRIFGOTO(result, _EXIT);

    result = CAPStack_Create(&hStack);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_GetLength(hConditionList, &nConditionSize);
    ERRIFGOTO(result, _EXIT_ERROR);

    //create dummy condition then push on to the stack as a bottom
    pstStackBottomCondition = (SCondition *)malloc(sizeof(SCondition));
    ERRMEMGOTO(pstStackBottomCondition, result, _EXIT_ERROR);

    pstStackBottomCondition->enOperator = OPERATOR_NONE;

    result = CAPStack_Push(hStack, (void *)pstStackBottomCondition);
    ERRIFGOTO(result, _EXIT_ERROR);

    for (Loop = 0; Loop < nConditionSize; Loop++)
    {
        SCondition* pstCondition = NULL;

        //get condition from the end of the linked list
        result = CAPLinkedList_Get(hConditionList, LINKED_LIST_OFFSET_FIRST, Loop, &pData);
        ERRIFGOTO(result, _EXIT_ERROR);

        pstCondition = (SCondition*)pData;

        if(pstCondition->enOperator != OPERATOR_AND && pstCondition->enOperator != OPERATOR_OR && pstCondition->enOperator != OPERATOR_LEFT_PARENTHESIS && pstCondition->enOperator != OPERATOR_RIGHT_PARENTHESIS)
        {
            result = CAPLinkedList_Add(hPostfixConditionList, LINKED_LIST_OFFSET_LAST, 0, pstCondition);
            ERRIFGOTO(result, _EXIT_ERROR);
        }
        else
        {
            // LEFT_PARENTHESIS
            if(pstCondition->enOperator == OPERATOR_LEFT_PARENTHESIS)
            {
                //Push on to stack if it is left parenthesis
                result = CAPStack_Push(hStack, (void *)pstCondition);
                ERRIFGOTO(result, _EXIT_ERROR);
            }
            // RIGHT_PARENTHESIS
            else if (pstCondition->enOperator == OPERATOR_RIGHT_PARENTHESIS)
            {
                cap_bool bLoop = TRUE; 

                //Pop and add to linked list until it meets left parenthesis
                while(bLoop)
                {
                    result = CAPStack_Pop(hStack, (void**) &pstTopCondition);
                    ERRIFGOTO(result, _EXIT_ERROR);
                    if(pstTopCondition->enOperator == OPERATOR_LEFT_PARENTHESIS)
                    {
                        bLoop = FALSE;
                    }
                    else
                    {
                        result = CAPLinkedList_Add(hPostfixConditionList, LINKED_LIST_OFFSET_LAST, 0, pstTopCondition);
                        ERRIFGOTO(result, _EXIT_ERROR);
                    }
                }    
            }
            // OTHER THINGS
            else
            {
                cap_bool bLoop = TRUE; 
                int nTopPrecedence = 0;
                int nCurPrecedence = 0;

                //get precedence of current operator
                result = getOperatorPrecedence(pstCondition->enOperator, &nCurPrecedence);
                ERRIFGOTO(result, _EXIT_ERROR);

                while(bLoop)
                {
                    result = CAPStack_Top(hStack, (void**)&pstTopCondition);
                    ERRIFGOTO(result, _EXIT_ERROR);

                    result = getOperatorPrecedence(pstTopCondition->enOperator, &nTopPrecedence);
                    ERRIFGOTO(result, _EXIT_ERROR);

                    if(nTopPrecedence >= nCurPrecedence)
                    {
                        result = CAPStack_Pop(hStack, (void**)&pstTopCondition);
                        ERRIFGOTO(result, _EXIT_ERROR);

                        result = CAPLinkedList_Add(hPostfixConditionList, LINKED_LIST_OFFSET_LAST, 0, pstTopCondition);
                        ERRIFGOTO(result, _EXIT_ERROR);
                    }
                    else
                    {
                        result = CAPStack_Push(hStack, (void *)pstCondition);
                        ERRIFGOTO(result, _EXIT_ERROR);

                        bLoop = FALSE;
                    }

                }
            }
        }
    }

    cap_bool bLoop = TRUE; 

    //Pop and add to linked list until it meets dummy on the stack
    while(bLoop)
    {
        result = CAPStack_Top(hStack, (void**)&pstTopCondition);
        ERRIFGOTO(result, _EXIT_ERROR);

        if(pstTopCondition->enOperator == OPERATOR_NONE)
        {
            result = CAPStack_Pop(hStack, (void**)&pstTopCondition);
            ERRIFGOTO(result, _EXIT_ERROR);

            //deallocate memory of bottom dummy condition
            SAFEMEMFREE(pstTopCondition);

            bLoop = FALSE;
        }
        else
        {
            result = CAPStack_Pop(hStack, (void**)&pstTopCondition);
            ERRIFGOTO(result, _EXIT_ERROR);

            result = CAPLinkedList_Add(hPostfixConditionList, LINKED_LIST_OFFSET_LAST, 0, pstTopCondition);
            ERRIFGOTO(result, _EXIT_ERROR);
        }
    }    

    *phPostfixConditionList = hPostfixConditionList;

_EXIT:
    CAPStack_Destroy(&hStack, NULL, NULL);
    return result;

_EXIT_ERROR:
    //    CAPLinkedList_Traverse(hPostfixConditionList, NULL, NULL);
    //Since we used pointers of conditions from condtion list in scenario structure, we should not free them from list or stack
    CAPLinkedList_Destroy(&hPostfixConditionList);
    CAPStack_Destroy(&hStack, NULL, NULL);
    return result;
}

    // DESTORY MALLOC DATA


// DESTORY "SThingKeywords"
static cap_result destroyThingKeywords(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    SThingKeywords *pstThing = (SThingKeywords*)pData;
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pstThing, NULL, result, ERR_CAP_INTERNAL_FAIL, _EXIT);

    if(pstThing->hValueHash != NULL)
    {
        CAPHash_Destroy(&pstThing->hValueHash, NULL, NULL);
    }

    if(pstThing->hFunctionHash != NULL)
    {
        CAPHash_Destroy(&pstThing->hFunctionHash, NULL, NULL);
    }

    SAFE_CAPSTRING_DELETE(pstThing->strThingRealId);

    SAFEMEMFREE(pstThing);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

// DESTROY "STeamBehavior"
static cap_result destroyThingVariableList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    SThingVariable *pstThingVar = (SThingVariable*)pData;
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pstThingVar, NULL, result, ERR_CAP_INTERNAL_FAIL, _EXIT);

    SAFE_CAPSTRING_DELETE(pstThingVar->strVarName);

    SAFEMEMFREE(pstThingVar);
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result destroyThingVariable(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_handle hList = pData;

    CAPLinkedList_Traverse(hList, destroyThingVariableList, NULL);
    CAPLinkedList_Destroy(&hList);

    return ERR_CAP_NOERROR;
}

// DESTROY "STeamBehavior"
static cap_result destroyTeamBehavior(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    STeamBehavior *pstBehav = (STeamBehavior*)pData;
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pstBehav, NULL, result, ERR_CAP_INTERNAL_FAIL, _EXIT);

    CAPHash_Destroy(&(pstBehav->hThingNameHash), destroyThingVariable, NULL);

    SAFEMEMFREE(pstBehav->pstAction);
    SAFEMEMFREE(pstBehav->pstListen);
    SAFEMEMFREE(pstBehav->pstReport);
    SAFE_CAPSTRING_DELETE(pstBehav->strTeamName);

    SAFEMEMFREE(pstBehav);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


// DESTROY "SExpression"
static cap_result destroyExp(SExpression *pstExp)
{
    if(pstExp != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstExp->strPrimaryIdentifier);
        SAFE_CAPSTRING_DELETE(pstExp->strSubIdentifier);
        SAFE_CAPSTRING_DELETE(pstExp->strStringValue);
    }

    return ERR_CAP_NOERROR;
}

static cap_result destroyExpList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    SExpression *pstExp = (SExpression*)pData;

    destroyExp(pstExp);
    SAFEMEMFREE(pstExp);

    return ERR_CAP_NOERROR;
}


static cap_result destroyStringList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_string strString = NULL;
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_DATA, _EXIT);

    strString = (cap_string) pData;

    SAFE_CAPSTRING_DELETE(strString);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


// DESTROY "SExecutionStatement"
static cap_result destroyExecStmt(SExecutionStatement *pstExecStmt)
{
    if(pstExecStmt != NULL)
    {
        if(pstExecStmt->hInputList != NULL)
        {
            CAPLinkedList_Traverse(pstExecStmt->hInputList, destroyExpList, NULL);
            CAPLinkedList_Destroy(&(pstExecStmt->hInputList));
        }

        if(pstExecStmt->hOutputList != NULL)
        {
            CAPLinkedList_Traverse(pstExecStmt->hOutputList, destroyStringList, NULL);
            CAPLinkedList_Destroy(&(pstExecStmt->hOutputList));
        }

        SAFE_CAPSTRING_DELETE(pstExecStmt->strPrimaryIdentifier);
        SAFE_CAPSTRING_DELETE(pstExecStmt->strSubIdentifier);
    }

    return ERR_CAP_NOERROR;
}


// DESTROY "SCondition"
static cap_result destroyCond(SCondition *pstCond)
{
    if(pstCond != NULL)
    {
        destroyExp(pstCond->pstLeftOperand);
        SAFEMEMFREE(pstCond->pstLeftOperand);
        destroyExp(pstCond->pstRightOperand);
        SAFEMEMFREE(pstCond->pstRightOperand);
    }

    return ERR_CAP_NOERROR;
}

static cap_result destroyCondList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    SCondition *pstCond = (SCondition*)pData;

    destroyCond(pstCond);

    SAFEMEMFREE(pstCond);

    return ERR_CAP_NOERROR;
}
// DESTORY "SConditionalStatement"

static cap_result destroyCondStmt(SConditionalStatement *pstCondStmt)
{
    if(pstCondStmt != NULL)
    {
        CAPLinkedList_Traverse(pstCondStmt->hConditionList, destroyCondList, NULL);
        CAPLinkedList_Destroy(&pstCondStmt->hConditionList);
    }

    return ERR_CAP_NOERROR;
}


// DESTROY "SExecutionNode"

static cap_result destroyExecutionNode(IN int nOffset, IN void *pData, IN void *pUserData)
{
    // DESTROY "pstStatementData"
    cap_result result = ERR_CAP_UNKNOWN;
    SExecutionNode *pstExecNode = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INTERNAL_FAIL, _EXIT);

    pstExecNode = (SExecutionNode *) pData;

    if(pstExecNode->enType == IF_STATEMENT || pstExecNode->enType == LOOP_STATEMENT ||
        pstExecNode->enType == WAIT_UNTIL_STATEMENT)
        // SConditionalStatement
    {
        SConditionalStatement *pstCondStmt = (SConditionalStatement*)pstExecNode->pstStatementData;

        destroyCondStmt(pstCondStmt);
        SAFEMEMFREE(pstCondStmt);
    }
    else
        // SExecutionStatement
    {
        SExecutionStatement *pstExecStmt = (SExecutionStatement*)pstExecNode->pstStatementData;

        destroyExecStmt(pstExecStmt);
        SAFEMEMFREE(pstExecStmt);
    }

    SAFEMEMFREE(pstExecNode);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
    // pstNextTrue, pstNextFalse 는 이 List의 다른 부분에서 destroy 된다.
}

// *********************************************************************
// SCENARIO API

static cap_result destroyInternalData(SScenario *pstScenario)
{
    cap_result result = ERR_CAP_NOERROR;
    // DESTROY "hGroupExecModelHash"
     CAPHash_Destroy(&(pstScenario->hGroupExecModelHash), destroyTeamBehavior, NULL);

     // DESTROY "hThingHash"
     CAPHash_Destroy(&(pstScenario->hThingHash), destroyThingKeywords, NULL);

     // DESTROY "hExecutionNodeList"
     CAPLinkedList_Traverse(pstScenario->hExecutionNodeList, destroyExecutionNode, NULL);
     CAPLinkedList_Destroy(&(pstScenario->hExecutionNodeList));

     result = CAPLinkedList_Create(&(pstScenario->hExecutionNodeList));
     ERRIFGOTO(result, _EXIT);
     result = CAPHash_Create(HASH_BUCKETS, &(pstScenario->hThingHash));
     ERRIFGOTO(result, _EXIT);
_EXIT:
     return result;
}


cap_result AppScriptModeler_Create(OUT cap_handle *phModel)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(phModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*)malloc(sizeof(SScenario));
    ERRMEMGOTO(pstScenario, result, _EXIT);

    pstScenario->strScenarioText = NULL;
    pstScenario->bNotLoaded = TRUE;
    pstScenario->hExecutionNodeList = NULL;
    pstScenario->hGroupExecModelHash = NULL;
    pstScenario->hLock = NULL;
    pstScenario->hThingHash = NULL;
    pstScenario->pstFinish = NULL;
    pstScenario->pstGlobalExecModel = NULL;
    pstScenario->strScenarioName = NULL;
    pstScenario->strErrorString = NULL;

    pstScenario->strErrorString = CAPString_New();
    ERRMEMGOTO(pstScenario->strErrorString, result, _EXIT);

    pstScenario->strScenarioText = CAPString_New();
    ERRMEMGOTO(pstScenario->strScenarioText, result, _EXIT);

    pstScenario->strScenarioName = CAPString_New();
    ERRMEMGOTO(pstScenario->strScenarioName, result, _EXIT);

    result = CAPThreadLock_Create(&(pstScenario->hLock));
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Create(HASH_BUCKETS, &(pstScenario->hThingHash));
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Create(&(pstScenario->hExecutionNodeList));
    ERRIFGOTO(result, _EXIT);

    pstScenario->pstGlobalExecModel = (SExecutionGraph *)malloc(sizeof(SExecutionGraph));
    ERRMEMGOTO(pstScenario->pstGlobalExecModel, result, _EXIT);
    pstScenario->pstGlobalExecModel->pGraphData = NULL;
    pstScenario->pstGlobalExecModel->pstCurrent = NULL;
    pstScenario->pstGlobalExecModel->pstRoot = NULL;

    pstScenario->pstFinish = (SExecutionNode*)malloc(sizeof(SExecutionNode));
    ERRMEMGOTO(pstScenario->pstFinish, result, _EXIT);
    
    pstScenario->pstFinish->enType = FINISH_STATEMENT;
    pstScenario->pstFinish->pstStatementData = NULL;
    pstScenario->pstFinish->pstNextTrue = NULL;
    pstScenario->pstFinish->pstNextFalse = NULL;
    pstScenario->pstFinish->nLineNo = -1;

    *phModel = pstScenario;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstScenario != NULL)
    {
        SAFEMEMFREE(pstScenario->pstGlobalExecModel);
        CAPLinkedList_Destroy(&(pstScenario->hExecutionNodeList));
        CAPHash_Destroy(&(pstScenario->hThingHash), NULL, NULL);
        CAPThreadLock_Destroy(&(pstScenario->hLock));
        SAFE_CAPSTRING_DELETE(pstScenario->strScenarioName);
        SAFE_CAPSTRING_DELETE(pstScenario->strScenarioText);
        SAFE_CAPSTRING_DELETE(pstScenario->strErrorString);
        SAFEMEMFREE(pstScenario);
    }
    return result;
}


static cap_result destroyBehaviorData(IN OUT SBehaviorData **ppstBehaviorData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SBehaviorData *pstBehaviorData = NULL;

    IFVARERRASSIGNGOTO(ppstBehaviorData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(*ppstBehaviorData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstBehaviorData = (SBehaviorData *) *ppstBehaviorData;

    CAPStack_Destroy(&(pstBehaviorData->hGlobalLastNodeStack), NULL, NULL);
    CAPStack_Destroy(&(pstBehaviorData->hGlobalConditionStack), NULL, NULL);
    CAPStack_Destroy(&(pstBehaviorData->hLastNodeStack), NULL, NULL);
    CAPStack_Destroy(&(pstBehaviorData->hConditionStack), NULL, NULL);
    CAPHash_Destroy(&(pstBehaviorData->hExecFailLabelGlobalHash), NULL, NULL);
    CAPHash_Destroy(&(pstBehaviorData->hLabelGlobalHash), NULL, NULL);
    CAPHash_Destroy(&(pstBehaviorData->hExecFailLabelHash), NULL, NULL);
    CAPHash_Destroy(&(pstBehaviorData->hLabelHash), NULL, NULL);
    SAFEMEMFREE(pstBehaviorData);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result createBehaviorData(OUT SBehaviorData **ppstBehaviorData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SBehaviorData *pstBehaviorData = NULL;

    pstBehaviorData = (SBehaviorData*)calloc(1, sizeof(SBehaviorData));
    ERRMEMGOTO(pstBehaviorData, result, _EXIT);

    result = CAPHash_Create(HASH_BUCKETS, &(pstBehaviorData->hLabelHash));
    ERRIFGOTO(result, _EXIT);
    result = CAPHash_Create(HASH_BUCKETS, &(pstBehaviorData->hExecFailLabelHash));
    ERRIFGOTO(result, _EXIT);
    result = CAPHash_Create(HASH_BUCKETS, &(pstBehaviorData->hLabelGlobalHash));
    ERRIFGOTO(result, _EXIT);
    result = CAPHash_Create(HASH_BUCKETS, &(pstBehaviorData->hExecFailLabelGlobalHash));
    ERRIFGOTO(result, _EXIT);
    result = CAPStack_Create(&(pstBehaviorData->hConditionStack));
    ERRIFGOTO(result, _EXIT);
    result = CAPStack_Create(&(pstBehaviorData->hLastNodeStack));
    ERRIFGOTO(result, _EXIT);
    result = CAPStack_Create(&(pstBehaviorData->hGlobalConditionStack));
    ERRIFGOTO(result, _EXIT);
    result = CAPStack_Create(&(pstBehaviorData->hGlobalLastNodeStack));
    ERRIFGOTO(result, _EXIT);

    *ppstBehaviorData = pstBehaviorData;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        destroyBehaviorData(&pstBehaviorData);
    }
    return result;
}


cap_result AppScriptModeler_Load(cap_handle hModel, cap_string strScenarioName, cap_string strText)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int yacc_result = 0;
    cap_string strScriptToParse = NULL;

    // setting tmp things
    result = createBehaviorData(&g_pstBehaviorData);
    ERRIFGOTO(result, _EXIT);

    g_pstScenario = (SScenario*)hModel;

    strScriptToParse = CAPString_New();
    ERRMEMGOTO(strScriptToParse, result, _EXIT);

    result = CAPString_Set(strScriptToParse, strText);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_Set(g_pstScenario->strScenarioText, strText);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_Set(g_pstScenario->strScenarioName, strScenarioName);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_Trim(strScriptToParse);
    ERRIFGOTO(result, _EXIT);

    new__scan_string(CAPString_LowPtr(strScriptToParse, NULL)); // for parsing from string

    printf("BEFORE PARSING\n\n");
    // parsing
    yacc_result = yyparse();
    if(yacc_result) {
        new_lex_destroy();
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    printf("AFTER PARSING\n\n");

    new_lex_destroy();

    g_pstScenario->bNotLoaded = FALSE;

    result = ERR_CAP_NOERROR;
_EXIT:
    destroyBehaviorData(&g_pstBehaviorData);
    if(result != ERR_CAP_NOERROR && g_pstScenario != NULL)
    {
        destroyInternalData(g_pstScenario);
    }
    SAFE_CAPSTRING_DELETE(strScriptToParse);
    return result;
}



cap_result AppScriptModeler_GetScenarioName(cap_handle hModel, cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    result = CAPString_Set(strScenarioName, pstScenario->strScenarioName);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetCurrent(cap_handle hModel, OUT SExecutionNode **ppstExecNode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppstExecNode, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    *ppstExecNode = pstScenario->pstGlobalExecModel->pstCurrent;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetThingRealId(cap_handle hModel, IN cap_string strVirtualThingId, OUT cap_string strReaThinglId)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;
    SThingKeywords *pstKeyword = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strVirtualThingId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    result = CAPHash_GetDataByKey(pstScenario->hThingHash, strVirtualThingId, (void **)&pstKeyword);
    ERRIFGOTO(result, _EXIT);

    if(CAPString_Length(pstKeyword->strThingRealId) <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    result = CAPString_Set(strReaThinglId, pstKeyword->strThingRealId);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



cap_result AppScriptModeler_ClearExecution(cap_handle hModel)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    pstScenario->pstGlobalExecModel->pstCurrent = pstScenario->pstGlobalExecModel->pstRoot;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_MoveToNext(cap_handle hModel, cap_bool bDirection, OUT SExecutionNode **ppstExecNode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;
    SExecutionNode *pstCurrent = NULL;
    SExecutionNode *pstNext = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppstExecNode, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    pstCurrent = pstScenario->pstGlobalExecModel->pstCurrent;

    if(pstCurrent->enType == FINISH_STATEMENT)
    {
        ERRASSIGNGOTO(result, ERR_CAP_END_OF_DATA, _EXIT);
    }

    if(pstCurrent->pstNextFalse == NULL)
    {
        pstNext = pstCurrent->pstNextTrue;
    }
    else
    {
        if(bDirection == TRUE)
        {
            pstNext = pstCurrent->pstNextTrue;
            if(pstNext == NULL)
            {
                ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            }
        }
        else
        {
            pstNext = pstCurrent->pstNextFalse;
        }
    }

    pstScenario->pstGlobalExecModel->pstCurrent = pstNext;

    *ppstExecNode = pstScenario->pstGlobalExecModel->pstCurrent;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

typedef struct _STraverseNodeCallbackData {
    CbFnExecutionNodeTraverse fnCallback;
    void *pUserData;
} STraverseNodeCallbackData;

static cap_result CALLBACK traverseExecutionNodeList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SExecutionNode *pstNode = NULL;
    STraverseNodeCallbackData *pstCallback = NULL;

    IFVARERRASSIGNGOTO(pData , NULL, result, ERR_CAP_INVALID_DATA, _EXIT);
    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_DATA, _EXIT);

    pstNode = (SExecutionNode *) pData;
    pstCallback = (STraverseNodeCallbackData *) pUserData;

    result = pstCallback->fnCallback(nOffset, pstNode, pstCallback->pUserData);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_TraverseExecutionNodeList(cap_handle hModel, IN CbFnExecutionNodeTraverse fnCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;
    STraverseNodeCallbackData stCallback;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(fnCallback, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    stCallback.fnCallback = fnCallback;
    stCallback.pUserData = pUserData;

    result = CAPLinkedList_Traverse(pstScenario->hExecutionNodeList, traverseExecutionNodeList, &stCallback);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetErrorInfo(cap_handle hModel, IN OUT cap_string strErrorInfo)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strErrorInfo, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    result = CAPString_Set(strErrorInfo, pstScenario->strErrorString);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetThingKeywordHash(cap_handle hModel, OUT cap_handle *phThingHash)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(hModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(phThingHash, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*) hModel;

    if(pstScenario->bNotLoaded == TRUE) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_INITIALIZED, _EXIT);
    }

    *phThingHash = pstScenario->hThingHash;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result ;
}


cap_result AppScriptModeler_Destroy(IN OUT cap_handle *phModel)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SScenario *pstScenario = NULL;

    IFVARERRASSIGNGOTO(phModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(*phModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstScenario = (SScenario*)*phModel;

    // DESTROY "pstGlobalExecModel"
   // SAFEMEMFREE(pstScenario->pstGlobalExecModel);

    // DESTROY "hGroupExecModelHash"
    CAPHash_Destroy(&(pstScenario->hGroupExecModelHash), destroyTeamBehavior, NULL);
    
    // DESTROY "hThingHash"
    CAPHash_Destroy(&(pstScenario->hThingHash), destroyThingKeywords, NULL);

    // DESTROY "hExecutionNodeList"
    CAPLinkedList_Traverse(pstScenario->hExecutionNodeList, destroyExecutionNode, NULL);
    CAPLinkedList_Destroy(&(pstScenario->hExecutionNodeList));

    CAPThreadLock_Destroy(&(pstScenario->hLock));

    // DESTROY "pstFinish"
    SAFEMEMFREE(pstScenario->pstFinish);

    // DESTROY "strScenarioText"
    SAFE_CAPSTRING_DELETE(pstScenario->strScenarioText);
    // DESTROY "strScenarioName"
    SAFE_CAPSTRING_DELETE(pstScenario->strScenarioName);
    SAFE_CAPSTRING_DELETE(pstScenario->strErrorString);

    SAFEMEMFREE(pstScenario->pstGlobalExecModel);

    SAFEMEMFREE(pstScenario);

    *phModel = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



