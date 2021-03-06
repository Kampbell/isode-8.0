
/*  A Bison parser, made from config.b  */

#define	SEARCHPATH	258
#define	NUMBER	259
#define	STRING	260

#line 1 "config.b"

/* $Header: /xtel/isode/isode/others/quipu/uips/doog/RCS/config.parse.c,v 9.0 1992/06/16 12:45:18 isode Rel $ */

/*
 * $Log: config.parse.c,v $
 * Revision 9.0  1992/06/16  12:45:18  isode
 * Release 8.0
 *
 */

#include "types.h"
#include "sequence.h"
#include "util.h"
#include "initialize.h"
#include "query.h"

#include "quipu/util.h"

#ifdef _AIX
#pragma alloca
#endif

#define yyparse parseConfig
static int yylex();

void add_ufn_path_element();

extern FILE *config_file;
extern QCardinal config_line_num;
extern char config_file_name[];

static char lexeme[255];

void exit();


#line 40 "config.b"
typedef union {
	int number;
	QCardinal symbol;
	char strval[255];
	entryList dnlist;
} YYSTYPE;

#ifndef YYLTYPE
typedef
struct yyltype {
	int timestamp;
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	char *text;
}
yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		29
#define	YYFLAG		-32768
#define	YYNTBASE	11

#define YYTRANSLATE(x) ((unsigned)(x) <= 260 ? yytranslate[x] : 17)

static const char yytranslate[] = {     0,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     7,
										8,     2,    10,     9,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     6,     2,     2,
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
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
										2,     2,     2,     2,     2,     1,     2,     3,     4,     5
								  };

static const short yyprhs[] = {     0,
									0,     2,     5,     6,    14,    24,    34,    36,    38,    42
							  };

static const short yyrhs[] = {    12,
								  0,    13,    12,     0,     0,     3,     6,    14,     6,     7,
								  16,     8,     0,     3,     6,    14,     9,    15,     6,     7,
								  16,     8,     0,     3,     6,    14,     9,    10,     6,     7,
								  16,     8,     0,     4,     0,     4,     0,     5,     9,    16,
								  0,     5,     0
							 };

#if YYDEBUG != 0
static const short yyrline[] = { 0,
								 59,    62,    63,    66,    71,    76,    82,    85,    88,    93
							   };

static const char * const yytname[] = {   "$",
										  "error","$illegal.","SEARCHPATH","NUMBER","STRING","':'","'('","')'","','","'+'",
										  "configure","option_list","option","lower_bound","upper_bound","entry_list",""
									  };
#endif

static const short yyr1[] = {     0,
								  11,    12,    12,    13,    13,    13,    14,    15,    16,    16
							};

static const short yyr2[] = {     0,
								  1,     2,     0,     7,     9,     9,     1,     1,     3,     1
							};

static const short yydefact[] = {     3,
									  0,     1,     3,     0,     2,     7,     0,     0,     0,     0,
									  8,     0,     0,    10,     0,     0,     0,     0,     4,     0,
									  0,     9,     0,     0,     6,     5,     0,     0,     0
								};

static const short yydefgoto[] = {    27,
									  2,     3,     7,    13,    15
								 };

static const short yypact[] = {     4,
									3,-32768,     4,    -2,-32768,-32768,    -1,     5,    -4,     6,
									-32768,     7,     8,     1,     9,    11,    12,     6,-32768,     6,
									6,-32768,    13,    14,-32768,-32768,    15,    16,-32768
							  };

static const short yypgoto[] = {-32768,
								17,-32768,-32768,-32768,   -17
							   };


#define	YYLAST		22


static const short yytable[] = {    11,
									22,     6,    23,    24,     8,    12,     1,     9,     4,    18,
									14,    10,    16,    17,    28,    29,    19,    20,    21,     5,
									25,    26
							   };

static const short yycheck[] = {     4,
									 18,     4,    20,    21,     6,    10,     3,     9,     6,     9,
									 5,     7,     6,     6,     0,     0,     8,     7,     7,     3,
									 8,     8
							   };
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* Not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__)
#include <alloca.h>
#else /* Not sparc */
#ifdef MSDOS
#include <malloc.h>
#endif /* MSDOS */
#endif /* Not sparc.  */
#endif /* Not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		yylex(&yylval, &yylloc)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
char *from;
char *to;
int count;
{
	char *f = from;
	char *t = to;
	int i = count;

	while (i-- > 0)
		*t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (char *from, char *to, int count) {
	char *f = from;
	char *t = to;
	int i = count;

	while (i-- > 0)
		*t++ = *f++;
}

#endif

#line 160 "/usr/local/lib/bison.simple"
int
yyparse() {
	int yystate;
	int yyn;
	short *yyssp;
	YYSTYPE *yyvsp;
	int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
	int yychar1;		/*  lookahead token as an internal (translated) token number */

	short	yyssa[YYINITDEPTH];	/*  the state stack			*/
	YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

	short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
	YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
	YYLTYPE *yyls = yylsa;
	YYLTYPE *yylsp;
	YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/

#define YYPOPSTACK   (yyvsp--, yysp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yysp--)
#endif

	int yystacksize = YYINITDEPTH;

#ifdef YYPURE
	int yychar;
	YYSTYPE yylval;
	int yynerrs;
#ifdef YYLSP_NEEDED
	YYLTYPE yylloc;
#endif
#endif

	YYSTYPE yyval;		/*  the variable used to return		*/
	/*  semantic values from the action	*/
	/*  routines				*/

	int yylen;

#if YYDEBUG != 0
	if (yydebug)
		fprintf(stderr, "Starting parse\n");
#endif

	yystate = 0;
	yyerrstatus = 0;
	yynerrs = 0;
	yychar = YYEMPTY;		/* Cause a token to be read.  */

	/* Initialize stack pointers.
	   Waste one element of value and location stack
	   so that they stay on the same level as the state stack.  */

	yyssp = yyss - 1;
	yyvsp = yyvs;
#ifdef YYLSP_NEEDED
	yylsp = yyls;
#endif

	/* Push a new state, which is found in  yystate  .  */
	/* In all cases, when you get here, the value and location stacks
	   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

	*++yyssp = yystate;

	if (yyssp >= yyss + yystacksize - 1) {
		/* Give user a chance to reallocate the stack */
		/* Use copies of these so that the &'s don't force the real ones into memory. */
		YYSTYPE *yyvs1 = yyvs;
		short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
		YYLTYPE *yyls1 = yyls;
#endif

		/* Get the current used size of the three stacks, in elements.  */
		int size = yyssp - yyss + 1;

#ifdef yyoverflow
		/* Each stack pointer address is followed by the size of
		the data in use in that stack, in bytes.  */
		yyoverflow("parser stack overflow",
				   &yyss1, size * sizeof (*yyssp),
				   &yyvs1, size * sizeof (*yyvsp),
#ifdef YYLSP_NEEDED
				   &yyls1, size * sizeof (*yylsp),
#endif
				   &yystacksize);

		yyss = yyss1;
		yyvs = yyvs1;
#ifdef YYLSP_NEEDED
		yyls = yyls1;
#endif
#else /* no yyoverflow */
		/* Extend the stack our own way.  */
		if (yystacksize >= YYMAXDEPTH) {
			yyerror("parser stack overflow");
			return 2;
		}
		yystacksize *= 2;
		if (yystacksize > YYMAXDEPTH)
			yystacksize = YYMAXDEPTH;
		yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
		__yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
		yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
		__yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
		yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
		__yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

		yyssp = yyss + size - 1;
		yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
		yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
		if (yydebug)
			fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

		if (yyssp >= yyss + yystacksize - 1)
			YYABORT;
	}

#if YYDEBUG != 0
	if (yydebug)
		fprintf(stderr, "Entering state %d\n", yystate);
#endif

yybackup:

	/* Do appropriate processing given the current state.  */
	/* Read a lookahead token if we need one and don't already have one.  */
	/* yyresume: */

	/* First try to decide what to do without reference to lookahead token.  */

	yyn = yypact[yystate];
	if (yyn == YYFLAG)
		goto yydefault;

	/* Not known => get a lookahead token if don't already have one.  */

	/* yychar is either YYEMPTY or YYEOF
	   or a valid token in external form.  */

	if (yychar == YYEMPTY) {
#if YYDEBUG != 0
		if (yydebug)
			fprintf(stderr, "Reading a token: ");
#endif
		yychar = YYLEX;
	}

	/* Convert token to internal form (in yychar1) for indexing tables with */

	if (yychar <= 0) {	/* This means end of input. */
		yychar1 = 0;
		yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
		if (yydebug)
			fprintf(stderr, "Now at end of input.\n");
#endif
	} else {
		yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
		if (yydebug)
			fprintf(stderr, "Next token is %d (%s)\n", yychar, yytname[yychar1]);
#endif
	}

	yyn += yychar1;
	if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
		goto yydefault;

	yyn = yytable[yyn];

	/* yyn is what to do for this token type in this state.
	   Negative => reduce, -yyn is rule number.
	   Positive => shift, yyn is new state.
	     New state is final state => don't bother to shift,
	     just return success.
	   0, or most negative number => error.  */

	if (yyn < 0) {
		if (yyn == YYFLAG)
			goto yyerrlab;
		yyn = -yyn;
		goto yyreduce;
	} else if (yyn == 0)
		goto yyerrlab;

	if (yyn == YYFINAL)
		YYACCEPT;

	/* Shift the lookahead token.  */

#if YYDEBUG != 0
	if (yydebug)
		fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

	/* Discard the token being shifted unless it is eof.  */
	if (yychar != YYEOF)
		yychar = YYEMPTY;

	*++yyvsp = yylval;
#ifdef YYLSP_NEEDED
	*++yylsp = yylloc;
#endif

	/* count tokens shifted since error; after three, turn off error status.  */
	if (yyerrstatus) yyerrstatus--;

	yystate = yyn;
	goto yynewstate;

	/* Do the default action for the current state.  */
yydefault:

	yyn = yydefact[yystate];
	if (yyn == 0)
		goto yyerrlab;

	/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
	yylen = yyr2[yyn];
	yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
	if (yydebug) {
		int i;

		fprintf (stderr, "Reducing via rule %d (line %d), ",
				 yyn, yyrline[yyn]);

		/* Print the symboles being reduced, and their result.  */
		for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
			fprintf (stderr, "%s ", yytname[yyrhs[i]]);
		fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
	}
#endif


	switch (yyn) {

	case 4:
#line 67 "config.b"
		{
			add_ufn_path_element(yyvsp[-4].number, UfnNobound, yyvsp[-1].dnlist);
			;
			break;
		}
	case 5:
#line 72 "config.b"
		{
			add_ufn_path_element(yyvsp[-6].number, yyvsp[-4].number, yyvsp[-1].dnlist);
			;
			break;
		}
	case 6:
#line 77 "config.b"
		{
			add_ufn_path_element(yyvsp[-6].number, UfnPlus, yyvsp[-1].dnlist);
			;
			break;
		}
	case 7:
#line 82 "config.b"
		{
			yyval.number = yyvsp[0].number;;
			break;
		}
	case 8:
#line 85 "config.b"
		{
			yyval.number = yyvsp[0].number;;
			break;
		}
	case 9:
#line 89 "config.b"
		{
			yyval.dnlist = yyvsp[0].dnlist;
			dn_list_insert(yyvsp[-2].strval, &yyval.dnlist, NULLAttrT);
			;
			break;
		}
	case 10:
#line 94 "config.b"
		{
			yyval.dnlist = NULLEntryList;
			dn_list_insert(yyvsp[0].strval, &yyval.dnlist, NULLAttrT);
			;
			break;
		}
	}
	/* the action file gets copied in in place of this dollarsign */
#line 423 "/usr/local/lib/bison.simple"
	
	yyvsp -= yylen;
	yyssp -= yylen;
#ifdef YYLSP_NEEDED
	yylsp -= yylen;
#endif

#if YYDEBUG != 0
	if (yydebug) {
		short *ssp1 = yyss - 1;
		fprintf (stderr, "state stack now");
		while (ssp1 != yyssp)
			fprintf (stderr, " %d", *++ssp1);
		fprintf (stderr, "\n");
	}
#endif

	*++yyvsp = yyval;

#ifdef YYLSP_NEEDED
	yylsp++;
	if (yylen == 0) {
		yylsp->first_line = yylloc.first_line;
		yylsp->first_column = yylloc.first_column;
		yylsp->last_line = (yylsp-1)->last_line;
		yylsp->last_column = (yylsp-1)->last_column;
		yylsp->text = 0;
	} else {
		yylsp->last_line = (yylsp+yylen-1)->last_line;
		yylsp->last_column = (yylsp+yylen-1)->last_column;
	}
#endif

	/* Now "shift" the result of the reduction.
	   Determine what state that goes to,
	   based on the state we popped back to
	   and the rule number reduced by.  */

	yyn = yyr1[yyn];

	yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
	if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
		yystate = yytable[yystate];
	else
		yystate = yydefgoto[yyn - YYNTBASE];

	goto yynewstate;

yyerrlab:   /* here on detecting error */

	if (! yyerrstatus)
		/* If not already recovering from an error, report this error.  */
	{
		++yynerrs;

#ifdef YYERROR_VERBOSE
		yyn = yypact[yystate];

		if (yyn > YYFLAG && yyn < YYLAST) {
			int size = 0;
			char *msg;
			int x, count;

			count = 0;
			for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
				if (yycheck[x + yyn] == x)
					size += strlen(yytname[x]) + 15, count++;
			msg = (char *) xmalloc(size + 15);
			strcpy(msg, "parse error");

			if (count < 5) {
				count = 0;
				for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
					if (yycheck[x + yyn] == x) {
						strcat(msg, count == 0 ? ", expecting `" : " or `");
						strcat(msg, yytname[x]);
						strcat(msg, "'");
						count++;
					}
			}
			yyerror(msg);
			free(msg);
		} else
#endif /* YYERROR_VERBOSE */
			yyerror("parse error");
	}

yyerrlab1:   /* here on error raised explicitly by an action */

	if (yyerrstatus == 3) {
		/* if just tried and failed to reuse lookahead token after an error, discard it.  */

		/* return failure if at end of input */
		if (yychar == YYEOF)
			YYABORT;

#if YYDEBUG != 0
		if (yydebug)
			fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

		yychar = YYEMPTY;
	}

	/* Else will try to reuse lookahead token
	   after shifting the error token.  */

	yyerrstatus = 3;		/* Each real token shifted decrements this */

	goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
	/* This is wrong; only states that explicitly want error tokens
	   should shift them.  */
	yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
	if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

	if (yyssp == yyss) YYABORT;
	yyvsp--;
	yystate = *--yyssp;
#ifdef YYLSP_NEEDED
	yylsp--;
#endif

#if YYDEBUG != 0
	if (yydebug) {
		short *ssp1 = yyss - 1;
		fprintf (stderr, "Error: state stack now");
		while (ssp1 != yyssp)
			fprintf (stderr, " %d", *++ssp1);
		fprintf (stderr, "\n");
	}
#endif

yyerrhandle:

	yyn = yypact[yystate];
	if (yyn == YYFLAG)
		goto yyerrdefault;

	yyn += YYTERROR;
	if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
		goto yyerrdefault;

	yyn = yytable[yyn];
	if (yyn < 0) {
		if (yyn == YYFLAG)
			goto yyerrpop;
		yyn = -yyn;
		goto yyreduce;
	} else if (yyn == 0)
		goto yyerrpop;

	if (yyn == YYFINAL)
		YYACCEPT;

#if YYDEBUG != 0
	if (yydebug)
		fprintf(stderr, "Shifting error token, ");
#endif

	*++yyvsp = yylval;
#ifdef YYLSP_NEEDED
	*++yylsp = yylloc;
#endif

	yystate = yyn;
	goto yynewstate;
}
#line 100 "config.b"


static int
yylex (YYSTYPE *lvalp, YYLTYPE *llocp) {
	int c;
	QCardinal count = 0;
	FILE *lexfile = config_file;

	lexeme[0] = '\0';

	while (isspace(c = getc(lexfile))) {
		if (c == EOF) return 0;
		if (c == '\n') config_line_num++;
	}

	lexeme[count++] = c;

	switch (c) {
	case '#':
		while ((c = getc(lexfile)) != '\n' && c != EOF)
			;

		config_line_num++;

		return yylex(lvalp, llocp);

	case ':':
	case '(':
	case ')':
	case ',':
	case '+':
		lexeme[count] = '\0';
		return c;

	case '\"':
		count = 0;

		c = getc(lexfile);

		while (c != EOF && c != '\"') {
			lexeme[count++] = (char) c;
			c = getc(lexfile);
		}

		if (c == '\"' || c == '\0') {
			lexeme[count] = '\0';
			strcpy(lvalp->strval, lexeme);

			return STRING;
		}

		break;
	}

	while ((c = getc(lexfile)) != EOF &&
			!(c == '('
			  || c == ')'
			  || c == ':'
			  || c == ','
			  || c == '\"'
			  || isspace((char) c)))
		lexeme[count++] = (char) c;

	if (c == EOF) return 0;

	fseek(lexfile, (long) -1, 1);

	lexeme[count] = '\0';

	switch (lexeme[0]) {
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '0':
		lvalp->number = (int) atoi(lexeme);
		return NUMBER;

	case 's':
	case 'S':
		if (!lexequ(lexeme, "searchpath")) {
			lvalp->symbol = SEARCHPATH;
			return SEARCHPATH;
		}

		break;
	}

	return 0;
}

/* ARGSUSED */
int
yyerror (char *err) {
	fprintf(stderr, "Line %d. ", config_line_num);
	fprintf(stderr,
			"Parse error in file `%s' at or near `%s'.\n",
			config_file_name,
			lexeme);
	exit(1);
}

