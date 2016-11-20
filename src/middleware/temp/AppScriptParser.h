/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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
