/* asn_ext.c - ASN.1 code pepsy can't quite do yet */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/x500as/RCS/asn_ext.c,v 9.0 1992/06/16 12:14:33 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/x500as/RCS/asn_ext.c,v 9.0 1992/06/16 12:14:33 isode Rel $
 *
 *
 * $Log: asn_ext.c,v $
 * Revision 9.0  1992/06/16  12:14:33  isode
 * Release 8.0
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include "quipu/util.h"
#include "quipu/entry.h"
#include "quipu/common.h"
#include "quipu/dsargument.h"
#include "quipu/turbo.h"
#include "IF-types.h"
#include "Quipu-types.h"

#define	advise	PY_advise

extern	LLog	* log_dsap;


/* Encoding substring filters */
/*
 *
 *        substrings
 *                %{
 *                        subs_temp = &(parm->UNSUB);
 *			subs_type = 1;
 *			if((avs_temp = subs_temp->fi_sub_initial) == NULLAV)
 *			{
 *			    ++subs_type;
 *			    if((avs_temp = subs_temp->fi_sub_any) == NULLAV)
 *			    {
 *				++subs_type;
 *				avs_temp = subs_temp->fi_sub_final;
 *			    }
 *			}
 *                %}
 *                [1] SEQUENCE
 *                {
 *                type
 *                        AttributeType [[p subs_temp->fi_sub_type]]
 *                        %{
 *                        %} ,
 *                strings
 *                        SEQUENCE OF
 *                        %{
 *                        %}
 *			<<; avs_temp != NULLAV;>>
 *                                CHOICE
 *                                <<subs_type>>
 *                                {
 *                                initial
 *                                        [0] AttributeValue [[p &avs_temp->avseq_av]]
 *                                        %{
 *						if((avs_temp = avs_temp->avseq_next) == NULLAV)
 *						{
 *						    ++subs_type;
 *						    if((avs_temp = subs_temp->fi_sub_any) == NULLAV)
 *						    {
 *							++subs_type;
 *							avs_temp = subs_temp->fi_sub_final;
 *						    }
 *						}
 *                                        %} ,
 *                                any
 *                                        [1] AttributeValue [[p &avs_temp->avseq_av]]
 *                                        %{
 *						if((avs_temp = avs_temp->avseq_next) == NULLAV)
 *						{
 *						    ++subs_type;
 *						    avs_temp = subs_temp->fi_sub_final;
 *						}
 *                                        %} ,
 *                                final
 *                                        [2] AttributeValue [[p &avs_temp->avseq_av]]
 *                                        %{
 *						avs_temp = avs_temp->avseq_next;
 *                                        %}
 *                                }
 *                                %{
 *                                %}
 *                        %{
 *                        %}
 *                }
 *
 */


int
substring_encode (struct filter_item *parm, PE *pe) {
	int		subs_type;
	AV_Sequence	avs_temp;
	Filter_Substrings	*subs_temp;

	PE	p91_z = NULLPE;
	PE *p91 = &p91_z;

	if (((*pe) = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SEQ)) == NULLPE) {
		advise (NULLCP, "substrings: %s", PEPY_ERR_NOMEM);
		return NOTOK;
	}

	subs_temp = &(parm->UNSUB);
	subs_type = 1;
	if((avs_temp = subs_temp->fi_sub_initial) == NULLAV) {
		++subs_type;
		if((avs_temp = subs_temp->fi_sub_any) == NULLAV) {
			++subs_type;
			avs_temp = subs_temp->fi_sub_final;
		}
	}

	(*p91) = NULLPE;

	if (encode_IF_AttributeType (p91, 0, 0, NULLCP, subs_temp->fi_sub_type) == NOTOK)
		return NOTOK;


	if ((*p91) != NULLPE)
		if (seq_add ((*pe), (*p91), -1) == NOTOK) {
			advise (NULLCP, "substrings %s%s", PEPY_ERR_BAD_SEQ,
					pe_error ((*pe) -> pe_errno));
			return NOTOK;
		}
	(*p91) = NULLPE;

	{
		/* strings */
		PE	p92 = NULLPE;
		PE	p93_z = NULLPE;
		PE *p93 = &p93_z;

		if (((*p91) = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SEQ)) == NULLPE) {
			advise (NULLCP, "strings: %s", PEPY_ERR_NOMEM);
			return NOTOK;
		}
		for (; avs_temp != NULLAV;) {
			{
				int	p94;

				switch (p94 = (subs_type)) {
				case 1: {	/* initial */
					if (encode_IF_AttributeValue (p93, 0, 0, NULLCP, &avs_temp->avseq_av) == NOTOK)
						return NOTOK;


					{

						if((avs_temp = avs_temp->avseq_next) == NULLAV) {
							++subs_type;
							if((avs_temp = subs_temp->fi_sub_any) == NULLAV) {
								++subs_type;
								avs_temp = subs_temp->fi_sub_final;
							}
						}

					}
					{
						/* initial TAG PUSHDOWN */
						PE p95_z;
						PE *p95 = &p95_z;

						if ((*p95 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0)) == NULLPE) {
							advise (NULLCP, "initial: %s", PEPY_ERR_NOMEM);
							return NOTOK;
						}
						(*p95) -> pe_cons = (*p93);
						(*p93) = *p95;
					}
				}
				break;
				case 2: {	/* any */
					if (encode_IF_AttributeValue (p93, 0, 0, NULLCP, &avs_temp->avseq_av) == NOTOK)
						return NOTOK;


					{

						if((avs_temp = avs_temp->avseq_next) == NULLAV) {
							++subs_type;
							avs_temp = subs_temp->fi_sub_final;
						}

					}
					{
						/* any TAG PUSHDOWN */
						PE p96_z;
						PE *p96 = &p96_z;

						if ((*p96 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 1)) == NULLPE) {
							advise (NULLCP, "any: %s", PEPY_ERR_NOMEM);
							return NOTOK;
						}
						(*p96) -> pe_cons = (*p93);
						(*p93) = *p96;
					}
				}
				break;
				case 3: {	/* final */
					if (encode_IF_AttributeValue (p93, 0, 0, NULLCP, &avs_temp->avseq_av) == NOTOK)
						return NOTOK;

					avs_temp = avs_temp->avseq_next;
					{
						/* final TAG PUSHDOWN */
						PE p97_z;
						PE *p97 = &p97_z;

						if ((*p97 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 2)) == NULLPE) {
							advise (NULLCP, "final: %s", PEPY_ERR_NOMEM);
							return NOTOK;
						}
						(*p97) -> pe_cons = (*p93);
						(*p93) = *p97;
					}
				}
				break;

				default:
					advise (NULLCP, "element %s%d", PEPY_ERR_INVALID_CHOICE,
							p94);
					return NOTOK;
				}


			}
			seq_addon ((*p91), p92, (*p93));
			p92 = (*p93);
		}

	}

	if ((*p91) != NULLPE)
		if (seq_add ((*pe), (*p91), -1) == NOTOK) {
			advise (NULLCP, "substrings %s%s", PEPY_ERR_BAD_SEQ,
					pe_error ((*pe) -> pe_errno));
			return NOTOK;
		}

	/*
	 *	{	/* substrings TAG PUSHDOWN */
	/*	    PE p98_z;
	 *	    PE *p98 = &p98_z;
	 *
	 *	    if ((*p98 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 1)) == NULLPE) {
	 *		advise (NULLCP, "substrings: %s", PEPY_ERR_NOMEM);
	 *		return NOTOK;
	 *	    }
	 *	    (*p98) -> pe_cons = (*pe);
	 *	    (*pe) = *p98;
	 *	}
	 */

	return OK;
}


/*  substring decoding
 *
 *        substrings
 *                %{
 *                        parm->fi_type = FILTERITEM_SUBSTRINGS;
 *                        subs_next = &(parm->fi_un.fi_un_substrings);
 *                        subs_next->fi_sub_initial = NULLAV;
 *                        subs_next->fi_sub_any = NULLAV;
 *                        subs_next->fi_sub_final = NULLAV;
 *			avs_initial = &(subs_next->fi_sub_initial);
 *			avs_any = &(subs_next->fi_sub_any);
 *			avs_final = &(subs_next->fi_sub_final);
 *                %}
 *                [1] SEQUENCE
 *                {
 *                type
 *                        AttributeType [[p &subs_next->fi_sub_type]]
 *                        %{
 *                        %} ,
 *                strings
 *                        SEQUENCE OF
 *                        %{
 *				av_next = AttrV_alloc();
 *                        %}
 *                                CHOICE
 *                                {
 *                                initial
 *                                        [0] AttributeValue [[p av_next]]
 *                                        %{
 *						if (AttrV_decode (subs_next->fi_sub_type,av_next) != OK) {
 *							LLOG (log_dsap,LLOG_EXCEPTIONS,("invalid initial value"));
 *							return NOTOK;
 *						}
 *                                                (*avs_initial) = avs_comp_new(av_next);
 *						avs_initial = &((*avs_initial)->avseq_next);
 *						(*avs_initial) = NULLAV;
 *                                        %} ,
 *                                any
 *                                        [1] AttributeValue [[p av_next]]
 *                                        %{
 *						if (AttrV_decode (subs_next->fi_sub_type,av_next) != OK) {
 *							LLOG (log_dsap,LLOG_EXCEPTIONS,("invalid any value"));
 *							return NOTOK;
 *						}
 *                                                (*avs_any) = avs_comp_new(av_next);
 *						avs_any = &((*avs_any)->avseq_next);
 *						(*avs_any) = NULLAV;
 *                                        %} ,
 *                                final
 *                                        [2] AttributeValue [[p av_next]]
 *                                        %{
 *						if (AttrV_decode (subs_next->fi_sub_type,av_next) != OK) {
 *							LLOG (log_dsap,LLOG_EXCEPTIONS,("invalid final value"));
 *							return NOTOK;
 *						}
 *                                                (*avs_final) = avs_comp_new(av_next);
 *						avs_final = &((*avs_final)->avseq_next);
 *						(*avs_final) = NULLAV;
 *                                        %}
 *                                }
 *                                %{
 *                                %}
 *                        %{
 *                        %}
 *                }
 *
 *
 */


int
substring_decode (struct filter_item **pparm, PE pe) {
	AV_Sequence	* avs_initial;
	AV_Sequence	* avs_any;
	AV_Sequence	* avs_final;
	AttributeValue	av_next;
	Filter_Substrings	*subs_next;
	struct filter_item *parm;

	PE p113 = pe;

	parm = *pparm;

#ifdef NOTANYMORE
	{
		/* substrings TAG PULLUP */
		PE p114;

		if ((p114 = prim2set (p113)) == NULLPE) {
			advise (NULLCP, "substrings %ssubstrings: %s", PEPY_ERR_BAD,
					pe_error (p113 -> pe_errno));
			return NOTOK;
		}
		if (p114 -> pe_cardinal != 1) {
			advise (NULLCP, "substrings %s substrings: %d", PEPY_ERR_TOO_MANY_TAGGED,
					p114 -> pe_cardinal);
			return NOTOK;
		}
		p113 = first_member (p114);

	}
#endif

	{
		PE p115;


		if (p113 -> pe_class != PE_CLASS_UNIV
				|| p113 -> pe_form != PE_FORM_CONS
				|| p113 -> pe_id != PE_CONS_SEQ) {
			advise (NULLCP, "substrings bad class/form/id: %s/%d/0x%x",
					pe_classlist[p113 -> pe_class], p113 -> pe_form, p113 -> pe_id);
			return NOTOK;
		}

		{
			parm->fi_type = FILTERITEM_SUBSTRINGS;
			subs_next = &(parm->fi_un.fi_un_substrings);
			subs_next->fi_sub_initial = NULLAV;
			subs_next->fi_sub_any = NULLAV;
			subs_next->fi_sub_final = NULLAV;
			avs_initial = &(subs_next->fi_sub_initial);
			avs_any = &(subs_next->fi_sub_any);
			avs_final = &(subs_next->fi_sub_final);

		}
		if ((p115 = prim2seq (p113)) == NULLPE) {
			advise (NULLCP, "substrings %s%s", PEPY_ERR_BAD_SEQ,
					pe_error (p113 -> pe_errno));
			return NOTOK;
		}
		p113 = p115;

		{
			PE p116;

			if ((p116 = first_member (p113)) != NULLPE) {
				p115 = p116;

				{
					/* type */
#ifdef DEBUG
					testdebug (p116, "type");
#endif

					if (decode_IF_AttributeType (p116, 1, (int *)0, NULLVP, &subs_next->fi_sub_type) == NOTOK)
						return NOTOK;
				}
			} else {
				advise (NULLCP, "substrings %stype element", PEPY_ERR_MISSING);
				return NOTOK;
			}

		}

		{
			PE p117;

			if ((p117 = (p113 != p115 ? next_member (p113, p115) : first_member (p113))) != NULLPE) {
				p115 = p117;

				{
					/* strings */
					PE p118;

#ifdef DEBUG
					testdebug (p117, "strings");
#endif

					if (p117 -> pe_class != PE_CLASS_UNIV
							|| p117 -> pe_form != PE_FORM_CONS
							|| p117 -> pe_id != PE_CONS_SEQ) {
						advise (NULLCP, "strings bad class/form/id: %s/%d/0x%x",
								pe_classlist[p117 -> pe_class], p117 -> pe_form, p117 -> pe_id);
						return NOTOK;
					}

					if ((p118 = prim2seq (p117)) == NULLPE) {
						advise (NULLCP, "strings %s%s", PEPY_ERR_BAD_SEQ,
								pe_error (p117 -> pe_errno));
						return NOTOK;
					}
					p117 = p118;

					for (p118 = first_member (p117); p118; p118 = next_member (p117, p118)) {
						{
#ifdef DEBUG
							testdebug (p118, "element");
#endif
							av_next = AttrV_alloc();
							av_next->av_syntax = 0;

							switch (PE_ID (p118 -> pe_class, p118 -> pe_id)) {
							case PE_ID (PE_CLASS_CONT, 0): {	/* initial */
								PE p119 = p118;

								{
									/* initial TAG PULLUP */
									PE p120;

									if ((p120 = prim2set (p119)) == NULLPE) {
										advise (NULLCP, "initial %sinitial: %s", PEPY_ERR_BAD,
												pe_error (p119 -> pe_errno));
										return NOTOK;
									}
									if (p120 -> pe_cardinal != 1) {
										advise (NULLCP, "initial %s initial: %d", PEPY_ERR_TOO_MANY_TAGGED,
												p120 -> pe_cardinal);
										return NOTOK;
									}
									p119 = first_member (p120);
								}
								{
#ifdef DEBUG
									testdebug (p119, "initial");
#endif

									/*
														if (decode_IF_AttributeValue (p119, 1, NULLINTP, NULLVP, &av_next) == NOTOK)
														    return NOTOK;
									*/
									av_next->av_struct = (caddr_t)p119;
									p119->pe_refcnt++;
									{

										if (AttrV_decode (subs_next->fi_sub_type,av_next) != OK) {
											LLOG (log_dsap,LLOG_EXCEPTIONS,("invalid initial value"));
											return NOTOK;
										}
										(*avs_initial) = avs_comp_new(av_next);
										avs_initial = &((*avs_initial)->avseq_next);
										(*avs_initial) = NULLAV;

									}
								}
							}
							break;
							case PE_ID (PE_CLASS_CONT, 1): {	/* any */
								PE p121 = p118;

								{
									/* any TAG PULLUP */
									PE p122;

									if ((p122 = prim2set (p121)) == NULLPE) {
										advise (NULLCP, "any %sany: %s", PEPY_ERR_BAD,
												pe_error (p121 -> pe_errno));
										return NOTOK;
									}
									if (p122 -> pe_cardinal != 1) {
										advise (NULLCP, "any %s any: %d", PEPY_ERR_TOO_MANY_TAGGED,
												p122 -> pe_cardinal);
										return NOTOK;
									}
									p121 = first_member (p122);
								}
								{
#ifdef DEBUG
									testdebug (p121, "any");
#endif
									/*
														if (decode_IF_AttributeValue (p121, 1, NULLINTP, NULLVP, &av_next) == NOTOK)
														    return NOTOK;
									*/
									av_next->av_struct = (caddr_t)p121;
									p121->pe_refcnt++;
									{

										if (AttrV_decode (subs_next->fi_sub_type,av_next) != OK) {
											LLOG (log_dsap,LLOG_EXCEPTIONS,("invalid any value"));
											return NOTOK;
										}
										(*avs_any) = avs_comp_new(av_next);
										avs_any = &((*avs_any)->avseq_next);
										(*avs_any) = NULLAV;

									}
								}
							}
							break;
							case PE_ID (PE_CLASS_CONT, 2): {	/* final */
								PE p123 = p118;

								{
									/* final TAG PULLUP */
									PE p124;

									if ((p124 = prim2set (p123)) == NULLPE) {
										advise (NULLCP, "final %sfinal: %s", PEPY_ERR_BAD,
												pe_error (p123 -> pe_errno));
										return NOTOK;
									}
									if (p124 -> pe_cardinal != 1) {
										advise (NULLCP, "final %s final: %d", PEPY_ERR_TOO_MANY_TAGGED,
												p124 -> pe_cardinal);
										return NOTOK;
									}
									p123 = first_member (p124);
								}
								{
#ifdef DEBUG
									testdebug (p123, "final");
#endif
									/*

														if (decode_IF_AttributeValue (p123, 1, NULLINTP, NULLVP, &av_next) == NOTOK)
														    return NOTOK;
									*/
									av_next->av_struct = (caddr_t)p123;
									p123->pe_refcnt++;

									{

										if (AttrV_decode (subs_next->fi_sub_type,av_next) != OK) {
											LLOG (log_dsap,LLOG_EXCEPTIONS,("invalid final value"));
											return NOTOK;
										}
										(*avs_final) = avs_comp_new(av_next);
										avs_final = &((*avs_final)->avseq_next);
										(*avs_final) = NULLAV;

									}
								}
							}
							break;

							default:
								advise (NULLCP, "element %s%s/%d/0x%x", PEPY_ERR_UNKNOWN_CHOICE,
										pe_classlist[p118 -> pe_class], p118 -> pe_form, p118 -> pe_id);
								return NOTOK;
							}
						}
					}
				}
			} else {
				advise (NULLCP, "substrings %sstrings element", PEPY_ERR_MISSING);
				return NOTOK;
			}

		}


		if (p113 -> pe_cardinal > 2) {
			advise (NULLCP, "substrings %s(2): %d", PEPY_ERR_TOO_MANY_ELEMENTS,
					p113 -> pe_cardinal);
			return NOTOK;
		}
	}
	return OK;
}

int
substring_free (struct filter_item *parm) {
	avs_free (parm->UNSUB.fi_sub_initial);
	avs_free (parm->UNSUB.fi_sub_any);
	avs_free (parm->UNSUB.fi_sub_final);

	return OK;
}



/* TreeStructureSyntax encode */
/*
 *
 *TreeStructureSyntax [[P struct tree_struct *]]
 *%{
 *OID     oid_tmp;
 *int	do_once;
 *%}
 *        ::=
 *        %{
 *                DLOG(log_dsap, LLOG_PDUS, ("About to encode TreeStructureSyntax"));
 *		if (parm->tree_object == NULLOBJECTCLASS) {
 *	                DLOG(log_dsap, LLOG_DEBUG, ("NULL OID in tree structure"));
 *			oid_tmp = NULLOID;
 *		} else {
 *	                oid_tmp = oid_cpy(parm->tree_object->oc_ot.ot_oid);
 *	                DLOG(log_dsap, LLOG_DEBUG, ("oc encodes as oid: %s", sprintoid(oid_tmp)));
 *		}
 *        %}
 *        SET
 *	{
 *	mandatoryObjectClasses
 *		[1] SET OF
 *		%{
 *	                DLOG(log_dsap, LLOG_DEBUG, ("Another mandatory oc"));
 *		%}
 *		<<do_once = 1; do_once != 0; do_once = 0>>
 *                	OBJECT IDENTIFIER [[O oid_tmp]] ,
 *	optionalObjectClasses
 *		[2] SET OF
 *                	OBJECT IDENTIFIER
 *		    -- OPTIONAL <<FALSE>> ,
 *		    OPTIONAL ,
 *	permittedRDNs
 *		[3] SET OF
 *			SET OF
 *				AttributeType [[p NULLAttrT]]
 *	}
 *        %{
 *		if(oid_tmp != NULLOID)
 *		    oid_free (oid_tmp);
 *                DLOG(log_dsap, LLOG_PDUS, ("Done encode TreeStructureSyntax"));
 *        %}
 *
 */

int
treestruct_encode (struct tree_struct *parm, PE *pe) {
	OID     oid_tmp;
	int	do_once;

	PE	p23_z = NULLPE;
	PE *p23 = &p23_z;

	if (((*pe) = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SET)) == NULLPE) {
		advise (NULLCP, "TreeStructureSyntax: %s", PEPY_ERR_NOMEM);
		return NOTOK;
	}
	{

		if (parm->tree_object == NULLOBJECTCLASS) {
			DLOG(log_dsap, LLOG_DEBUG, ("NULL OID in tree structure"));
			oid_tmp = NULLOID;
		} else {
			oid_tmp = oid_cpy(parm->tree_object->oc_ot.ot_oid);
			DLOG(log_dsap, LLOG_DEBUG, ("oc encodes as oid: %s", sprintoid(oid_tmp)));
		}

	}
	(*p23) = NULLPE;

	{
		PE	p24 = NULLPE;
		PE	p25_z = NULLPE;
		PE *p25 = &p25_z;

		if (((*p23) = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SET)) == NULLPE) {
			advise (NULLCP, "mandatoryObjectClasses: %s", PEPY_ERR_NOMEM);
			return NOTOK;
		}
		for (do_once = 1; do_once != 0; do_once = 0) {
			{

				DLOG(log_dsap, LLOG_DEBUG, ("Another mandatory oc"));

			}
			{
				OID p26;

				p26 = oid_tmp;
				if (p26 == NULLOID) {
					advise (NULLCP, "member %s", PEPY_ERR_INIT_FAILED);
					return NOTOK;
				}
				if (((*p25) = obj2prim (p26, PE_CLASS_UNIV, PE_PRIM_OID)) == NULLPE) {
					advise (NULLCP, "member: %s", PEPY_ERR_NOMEM);
					return NOTOK;
				}

#ifdef DEBUG
				testdebug ((*p25), "member");
#endif

			}
			set_addon ((*p23), p24, (*p25));
			p24 = (*p25);
		}

#ifdef DEBUG
		testdebug ((*p23), "mandatoryObjectClasses");
#endif

		{
			/* mandatoryObjectClasses TAG PUSHDOWN */
			PE p27_z;
			PE *p27 = &p27_z;

			if ((*p27 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 1)) == NULLPE) {
				advise (NULLCP, "mandatoryObjectClasses: %s", PEPY_ERR_NOMEM);
				return NOTOK;
			}
			(*p27) -> pe_cons = (*p23);
			(*p23) = *p27;
		}
	}
	if ((*p23) != NULLPE)
		if (set_add ((*pe), (*p23)) == NOTOK) {
			advise (NULLCP, "TreeStructureSyntax %s%s", PEPY_ERR_BAD_SET,
					pe_error ((*pe) -> pe_errno));
			return NOTOK;
		}
	(*p23) = NULLPE;

	if ((*p23) != NULLPE)
		if (set_add ((*pe), (*p23)) == NOTOK) {
			advise (NULLCP, "TreeStructureSyntax %s%s", PEPY_ERR_BAD_SET,
					pe_error ((*pe) -> pe_errno));
			return NOTOK;
		}
	(*p23) = NULLPE;

	{
		if (((*p23) = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SET)) == NULLPE) {
			advise (NULLCP, "permittedRDNs: %s", PEPY_ERR_NOMEM);
			return NOTOK;
		}

#ifdef DEBUG
		testdebug ((*p23), "permittedRDNs");
#endif

		{
			/* permittedRDNs TAG PUSHDOWN */
			PE p30_z;
			PE *p30 = &p30_z;

			if ((*p30 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 3)) == NULLPE) {
				advise (NULLCP, "permittedRDNs: %s", PEPY_ERR_NOMEM);
				return NOTOK;
			}
			(*p30) -> pe_cons = (*p23);
			(*p23) = *p30;
		}
	}
	if ((*p23) != NULLPE)
		if (set_add ((*pe), (*p23)) == NOTOK) {
			advise (NULLCP, "TreeStructureSyntax %s%s", PEPY_ERR_BAD_SET,
					pe_error ((*pe) -> pe_errno));
			return NOTOK;
		}

#ifdef DEBUG
	testdebug ((*pe), "Quipu.TreeStructureSyntax");
#endif

	{

		if(oid_tmp != NULLOID)
			oid_free (oid_tmp);

	}

	return OK;
}

/* TreeStructSyntax decode */
/*
 *
 *TreeStructureSyntax [[P struct tree_struct **]]
 *%{
 *AttributeType	  at_tmp;
 *OID     oid_tmp;
 *int	is_first;
 *objectclass * oc_add();
 *%}
 *        ::=
 *        %{
 *                DLOG(log_dsap, LLOG_PDUS, ("About to decode TreeStructureSyntax"));
 *		is_first = 1;
 *        %}
 *        SET
 *	{
 *	mandatoryObjectClasses
 *		[1] SET OF
 *                	OBJECT IDENTIFIER [[O oid_tmp]]
 *                	%{
 *				if(is_first != 0)
 *				{
 *                			(*parm) = tree_struct_alloc();
 *                        		if (((*parm)->tree_object = oid2oc(oid_tmp)) == NULLOBJECTCLASS)
 *						(*parm)->tree_object = oc_add (oid_tmp);
 *					is_first = 0;
 *				}
 *				else
 *				{
 *					LLOG(log_dsap, LLOG_EXCEPTIONS, ("Multiple mandatory object classes"));
 *				}
 *                	%} ,
 *	optionalObjectClasses
 *		[2] SET OF
 *                	OBJECT IDENTIFIER
 *		    OPTIONAL ,
 *	permittedRDNs
 *		[3] SET OF
 *			SET OF
 *				AttributeType [[p & at_tmp]]
 *				%{
 *				%}
 *	}
 *        %{
 *                DLOG(log_dsap, LLOG_PDUS, ("Done decode TreeStructureSyntax"));
 *        %}
 *
 *
 */


int
treestruct_decode (struct tree_struct **parm, PE pe) {

	AttributeType	  at_tmp;
	OID     oid_tmp;
	int	is_first;
	objectclass * oc_add();
	char explicit = 0;

	int p34_count = 0;
	PE p34;

#ifdef DEBUG
	testdebug (pe, "Quipu.TreeStructureSyntax");
#endif

	if (explicit) {
		if (pe -> pe_class != PE_CLASS_UNIV
				|| pe -> pe_form != PE_FORM_CONS
				|| pe -> pe_id != PE_CONS_SET) {
			advise (NULLCP, "TreeStructureSyntax bad class/form/id: %s/%d/0x%x",
					pe_classlist[pe -> pe_class], pe -> pe_form, pe -> pe_id);
			return NOTOK;
		}
	} else if (pe -> pe_form != PE_FORM_CONS) {
		advise (NULLCP, "TreeStructureSyntax bad form: %d", pe -> pe_form);
		return NOTOK;
	}

	{

		is_first = 1;

	}
	if ((p34 = prim2set (pe)) == NULLPE) {
		advise (NULLCP, "TreeStructureSyntax %s%s", PEPY_ERR_BAD_SET,
				pe_error (pe -> pe_errno));
		return NOTOK;
	}
	pe = p34;

	if (p34 = set_find (pe, PE_CLASS_CONT, 1)) {
		PE p35 = p34;

		{
			/* mandatoryObjectClasses TAG PULLUP */
			PE p36;

			if ((p36 = prim2set (p35)) == NULLPE) {
				advise (NULLCP, "mandatoryObjectClasses %smandatoryObjectClasses: %s", PEPY_ERR_BAD,
						pe_error (p35 -> pe_errno));
				return NOTOK;
			}
			if (p36 -> pe_cardinal != 1) {
				advise (NULLCP, "mandatoryObjectClasses %s mandatoryObjectClasses: %d", PEPY_ERR_TOO_MANY_TAGGED,
						p36 -> pe_cardinal);
				return NOTOK;
			}
			p35 = first_member (p36);
		}
		{
			PE p37;

#ifdef DEBUG
			testdebug (p35, "mandatoryObjectClasses");
#endif

			if (p35 -> pe_class != PE_CLASS_UNIV
					|| p35 -> pe_form != PE_FORM_CONS
					|| p35 -> pe_id != PE_CONS_SET) {
				advise (NULLCP, "mandatoryObjectClasses bad class/form/id: %s/%d/0x%x",
						pe_classlist[p35 -> pe_class], p35 -> pe_form, p35 -> pe_id);
				return NOTOK;
			}

			if ((p37 = prim2set (p35)) == NULLPE) {
				advise (NULLCP, "mandatoryObjectClasses %s%s", PEPY_ERR_BAD_SET,
						pe_error (p35 -> pe_errno));
				return NOTOK;
			}
			p35 = p37;

			for (p37 = first_member (p35); p37; p37 = next_member (p35, p37)) {
				OID p38;

#ifdef DEBUG
				testdebug (p37, "member");
#endif

				if (p37 -> pe_class != PE_CLASS_UNIV
						|| p37 -> pe_form != PE_FORM_PRIM
						|| p37 -> pe_id != PE_PRIM_OID) {
					advise (NULLCP, "member bad class/form/id: %s/%d/0x%x",
							pe_classlist[p37 -> pe_class], p37 -> pe_form, p37 -> pe_id);
					return NOTOK;
				}

				if ((p38 = prim2oid (p37)) == NULLOID) {
					advise (NULLCP, "member %s%s", PEPY_ERR_BAD_OID,
							pe_error (p37 -> pe_errno));
					return NOTOK;
				}

				/* Spurious copy noticed by Jim Reed
				                oid_tmp = oid_cpy (p38);
				*/
				oid_tmp = p38;
				{

					if(is_first != 0) {
						(*parm) = tree_struct_alloc();
						if (((*parm)->tree_object = oid2oc(oid_tmp)) == NULLOBJECTCLASS)
							(*parm)->tree_object = oc_add (oid_tmp);
						is_first = 0;
					} else {
						LLOG(log_dsap, LLOG_EXCEPTIONS, ("Multiple mandatory object classes"));
					}

				}
			}
		}
		p34_count ++;
	} else {
		advise (NULLCP, "mandatoryObjectClasses %s mandatoryObjectClasses member", PEPY_ERR_MISSING);
		return NOTOK;
	}

	if (p34 = set_find (pe, PE_CLASS_CONT, 2)) {
		PE p39 = p34;

		{
			/* optionalObjectClasses TAG PULLUP */
			PE p40;

			if ((p40 = prim2set (p39)) == NULLPE) {
				advise (NULLCP, "optionalObjectClasses %soptionalObjectClasses: %s", PEPY_ERR_BAD,
						pe_error (p39 -> pe_errno));
				return NOTOK;
			}
			if (p40 -> pe_cardinal != 1) {
				advise (NULLCP, "optionalObjectClasses %s optionalObjectClasses: %d", PEPY_ERR_TOO_MANY_TAGGED,
						p40 -> pe_cardinal);
				return NOTOK;
			}
			p39 = first_member (p40);
		}
		{
			PE p41;

#ifdef DEBUG
			testdebug (p39, "optionalObjectClasses");
#endif

			if (p39 -> pe_class != PE_CLASS_UNIV
					|| p39 -> pe_form != PE_FORM_CONS
					|| p39 -> pe_id != PE_CONS_SET) {
				advise (NULLCP, "optionalObjectClasses bad class/form/id: %s/%d/0x%x",
						pe_classlist[p39 -> pe_class], p39 -> pe_form, p39 -> pe_id);
				return NOTOK;
			}

			if ((p41 = prim2set (p39)) == NULLPE) {
				advise (NULLCP, "optionalObjectClasses %s%s", PEPY_ERR_BAD_SET,
						pe_error (p39 -> pe_errno));
				return NOTOK;
			}
			p39 = p41;

			for (p41 = first_member (p39); p41; p41 = next_member (p39, p41)) {
#ifdef DEBUG
				testdebug (p41, "member");
#endif

				if (p41 -> pe_class != PE_CLASS_UNIV
						|| p41 -> pe_form != PE_FORM_PRIM
						|| p41 -> pe_id != PE_PRIM_OID) {
					advise (NULLCP, "member bad class/form/id: %s/%d/0x%x",
							pe_classlist[p41 -> pe_class], p41 -> pe_form, p41 -> pe_id);
					return NOTOK;
				}

				if (prim2oid (p41) == NULLOID) {
					advise (NULLCP, "member %s%s", PEPY_ERR_BAD_OID,
							pe_error (p41 -> pe_errno));
					return NOTOK;
				}
			}
		}
		p34_count ++;
	}
	if (p34 = set_find (pe, PE_CLASS_CONT, 3)) {
		PE p42 = p34;

		{
			/* permittedRDNs TAG PULLUP */
			PE p43;

			if ((p43 = prim2set (p42)) == NULLPE) {
				advise (NULLCP, "permittedRDNs %spermittedRDNs: %s", PEPY_ERR_BAD,
						pe_error (p42 -> pe_errno));
				return NOTOK;
			}
			if (p43 -> pe_cardinal != 1) {
				advise (NULLCP, "permittedRDNs %s permittedRDNs: %d", PEPY_ERR_TOO_MANY_TAGGED,
						p43 -> pe_cardinal);
				return NOTOK;
			}
			p42 = first_member (p43);
		}
		{
			PE p44;

#ifdef DEBUG
			testdebug (p42, "permittedRDNs");
#endif

			if (p42 -> pe_class != PE_CLASS_UNIV
					|| p42 -> pe_form != PE_FORM_CONS
					|| p42 -> pe_id != PE_CONS_SET) {
				advise (NULLCP, "permittedRDNs bad class/form/id: %s/%d/0x%x",
						pe_classlist[p42 -> pe_class], p42 -> pe_form, p42 -> pe_id);
				return NOTOK;
			}

			if ((p44 = prim2set (p42)) == NULLPE) {
				advise (NULLCP, "permittedRDNs %s%s", PEPY_ERR_BAD_SET,
						pe_error (p42 -> pe_errno));
				return NOTOK;
			}
			p42 = p44;

			for (p44 = first_member (p42); p44; p44 = next_member (p42, p44)) {
				PE p45;

#ifdef DEBUG
				testdebug (p44, "member");
#endif

				if (p44 -> pe_class != PE_CLASS_UNIV
						|| p44 -> pe_form != PE_FORM_CONS
						|| p44 -> pe_id != PE_CONS_SET) {
					advise (NULLCP, "member bad class/form/id: %s/%d/0x%x",
							pe_classlist[p44 -> pe_class], p44 -> pe_form, p44 -> pe_id);
					return NOTOK;
				}

				if ((p45 = prim2set (p44)) == NULLPE) {
					advise (NULLCP, "member %s%s", PEPY_ERR_BAD_SET,
							pe_error (p44 -> pe_errno));
					return NOTOK;
				}
				p44 = p45;

				for (p45 = first_member (p44); p45; p45 = next_member (p44, p45)) {
#ifdef DEBUG
					testdebug (p45, "member");
#endif

					if (decode_IF_AttributeType (p45, 1, (int *)0, NULLVP, & at_tmp) == NOTOK)
						return NOTOK;
				}
			}
		}
		p34_count ++;
	} else {
		advise (NULLCP, "permittedRDNs %s permittedRDNs member", PEPY_ERR_MISSING);
		return NOTOK;
	}

	if (p34_count != pe -> pe_cardinal)
		advise (NULLCP, "%s", PEPY_ERR_EXTRA_MEMBERS);

	return OK;
}

int
EDB_encode (struct getedb_result *parm, PE *pe) {
	struct entry *ent_tmp;
	PE  p31 = NULLPE;
	PE  p32_z = NULLPE;
	PE *p32 = &p32_z;

	if (parm->gr_encoded) {
		*pe = parm->gr_pe;
		return OK;
	}

	if (((*pe) = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SEQ)) == NULLPE) {
		advise (NULLCP, "EntryDataBlock: %s", PEPY_ERR_NOMEM);
		return NOTOK;
	}


	for (ent_tmp = (Entry) avl_getfirst(parm->gr_edb);
			ent_tmp != NULLENTRY;
			ent_tmp = (Entry) avl_getnext()) {
		if ((ent_tmp -> e_data != E_DATA_MASTER) && (ent_tmp -> e_data != E_TYPE_SLAVE))
			continue;

		if (encode_Quipu_RelativeEntry (p32, 0, NULL, NULLCP, ent_tmp) == NOTOK)
			return NOTOK;

		seq_addon ((*pe), p31, (*p32));
		p31 = (*p32);
	}

	return OK;
}


int
EDB_decode_force (struct getedb_result **pparm, PE pe) {
	Avlnode **tree;
	struct entry *tmp;
	int	entry_cmp();
	PE p46;

	if (pe -> pe_class != PE_CLASS_UNIV
			|| pe -> pe_form != PE_FORM_CONS
			|| pe -> pe_id != PE_CONS_SEQ) {
		advise (NULLCP, "EntryDataBlock bad class/form/id: %s/%d/0x%x",
				pe_classlist[pe -> pe_class], pe -> pe_form, pe -> pe_id);
		return NOTOK;
	}

	if ((p46 = prim2seq (pe)) == NULLPE) {
		advise (NULLCP, "EntryDataBlock %s%s", PEPY_ERR_BAD_SEQ,
				pe_error (pe -> pe_errno));
		return NOTOK;
	}

	pe = p46;

	(*pparm)->gr_encoded = FALSE;
	(*pparm)->gr_edb = NULLAVL;
	tree = &((*pparm)->gr_edb);

	for (p46 = first_member (pe); p46; p46 = next_member (pe, p46)) {
		if (decode_Quipu_RelativeEntry (p46, 1, NULLIP, NULLVP, &tmp)== NOTOK)
			return NOTOK;

		tmp->e_leaf = TRUE;
		tmp->e_complete = TRUE;
		tmp->e_data = E_TYPE_SLAVE;
		if (avl_insert(tree, tmp, entry_cmp,
					   avl_dup_error) == NOTOK)
			LLOG(log_dsap, LLOG_EXCEPTIONS, ("Bad EDB update (contains duplicates)"));
	}


	return OK;
}


int
EDB_decode (struct getedb_result **pparm, PE pe) {

	if (pe -> pe_class != PE_CLASS_UNIV
			|| pe -> pe_form != PE_FORM_CONS
			|| pe -> pe_id != PE_CONS_SEQ) {
		advise (NULLCP, "EntryDataBlock bad class/form/id: %s/%d/0x%x",
				pe_classlist[pe -> pe_class], pe -> pe_form, pe -> pe_id);
		return NOTOK;
	}

	(*pparm)->gr_pe = pe;
	pe -> pe_refcnt++;

	(*pparm)->gr_encoded = TRUE;

	return OK;
}

int
EDB_free (struct getedb_result *parm) {
	/* All done for us at some other time (hopefully) */
	return OK;
}
