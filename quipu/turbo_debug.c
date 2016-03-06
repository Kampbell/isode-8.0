/* turbo_debug.c */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/quipu/RCS/turbo_debug.c,v 9.0 1992/06/16 12:34:01 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/quipu/RCS/turbo_debug.c,v 9.0 1992/06/16 12:34:01 isode Rel $
 *
 *
 * $Log: turbo_debug.c,v $
 * Revision 9.0  1992/06/16  12:34:01  isode
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


#include <sys/types.h>
#include <stdio.h>

#include "quipu/util.h"
#include "psap.h"
#include "quipu/commonarg.h"
#include "quipu/attr.h"
#include "quipu/entry.h"
#include "quipu/turbo.h"

extern LLog 		*log_dsap;
int		turbo_index_num;
AttributeType	*turbo_index;

PS	ps;

static rsavl_print( root, fn, fps, depth )
Avlnode	*root;
IFP	fn;
FILE	*fps;
int	depth;
{
	int	i;

	if ( root == 0 )
		return;

	rsavl_print( root->avl_right, fn, fps, depth+1 );

	for ( i = 0; i < depth; i++ )
		(*fn)( fps, "  " );
	(*fn)( fps, ((Index_node *)root->avl_data)->in_value );
	(*fn)( fps, " %d\n", root->avl_bf );

	rsavl_print( root->avl_left, fn, fps, depth+1 );
}

int 
savl_print (Avlnode *root)
{
#ifndef __STDC__
	int	fprintf ();
#else
#ifdef  __GNUC__
#ifdef  SUNOS4		/* And probaby a few more - what a mess! */
	int	fprintf ();
#endif
#endif
#endif

	 printf( "**** soundex avl_print ****\n" );

	if ( root == 0 ) {
		 printf( "NULL\n" );
		return;
	}

	( void ) rsavl_print( root, fprintf, (FILE *)stdout, 0 );

	 printf( "**** soundex avl_print end ****\n" );
}

static 
ravl_print (Avlnode *root, IFP fn, PS fps, int format, int depth)
{
	int	i;

	if ( root == 0 )
		return;

	ravl_print( root->avl_right, fn, fps, format, depth+1 );

	for ( i = 0; i < depth; i++ )
		ps_print( fps, "  " );
	(*fn)( fps, (( Entry )(root->avl_data))->e_name, format );
	ps_printf( fps, " %d\n", root->avl_bf );

	ravl_print( root->avl_left, fn, fps, format, depth+1 );
}

int 
avl_print (Avlnode *root)
{
	PS	fps;
	int	rdn_print();

	 printf( "**** avl_print ****\n" );

	if ( root == 0 ) {
		 printf( "NULL\n" );
		return;
	}

	if ( (fps = ps_alloc( std_open )) == NULLPS ) {
		 printf( "avl_print: ps_alloc failed\n" );
		return;
	}
	if ( std_setup( fps, stdout ) == NOTOK ) {
		 printf( "avl_print: std_setup failed\n" );
		return;
	}

	( void ) ravl_print( root, rdn_print, fps, EDBOUT, 0 );

	 printf( "**** avl_print end ****\n" );

	ps_free( fps );
}

static 
rprint_directory (Entry node, int depth)
{
	int	i;

	for ( i = 0; i < depth; i++ )
		ps_print( ps, "\t" );

	rdn_print( ps, node->e_name, EDBOUT );
	ps_print( ps, "\n" );

	if ( node->e_children != NULLAVL )
		 avl_apply( node->e_children, rprint_directory, (caddr_t) (depth + 1),
						  NOTOK, AVL_INORDER );
}

int 
print_directory (Entry node)
{
	if ( (ps = ps_alloc( std_open )) == NULLPS ) {
		 printf( "avl_print: ps_alloc failed\n" );
		return;
	}
	if ( std_setup( ps, stdout ) == NOTOK ) {
		 printf( "avl_print: std_setup failed\n" );
		return;
	}

	rprint_directory( node, 0 );

	ps_free( ps );
	 fflush( stdout );
}

/*
 * print_optimized_attrs -- print out a list of attributes being optimized.
 * for debugging...
 */

int 
print_optimized_attrs (void) {
	int	i;
	PS	fps;

	if ( (fps = ps_alloc( std_open )) == NULLPS ) {
		 printf( "turbo_index_print: ps_alloc failed\n" );
		return;
	}
	if ( std_setup( fps, stdout ) == NOTOK ) {
		 printf( "turbo_index_print: std_setup failed\n" );
		return;
	}

	ps_print( fps, "Optimized attributes:\n" );
	for ( i = 0; i < turbo_index_num; i++ ) {
		ps_printf( fps, "\t" );
		AttrT_print( fps, turbo_index[ i ], EDBOUT );
		ps_printf( fps, "\n" );
	}
	ps_printf( fps, "End of Optimized Attributes:\n" );

	ps_free( fps );
}

static 
print_index_node (Index_node *node, PS fps)
{
	int	i;

	ps_print( fps, "\t" );
	AttrV_print( fps, (AttributeValue)node->in_value, EDBOUT );
	ps_print( fps, "\n" );

	for ( i = 0; i < node->in_num; i++ ) {
		ps_print( fps, "\t\t" );
		rdn_print( fps, node->in_entries[ i ]->e_name,
				   EDBOUT );
		ps_print( fps, "\n" );
	}

	return( OK );
}

static 
print_soundex_node (Index_node *node, PS fps)
{
	int	i;

	ps_print( fps, "\t" );
	ps_print( fps, node->in_value );
	ps_print( fps, "\n" );

	for ( i = 0; i < node->in_num; i++ ) {
		ps_print( fps, "\t\t" );
		rdn_print( fps, node->in_entries[ i ]->e_name,
				   EDBOUT );
		ps_print( fps, "\n" );
	}

	return( OK );
}

/*
 * print_index -- print the given attribute index.
 */

print_index( pindex )
Index	*pindex;
{
	PS	fps;
	int	i;

	if ( pindex == (Index *) 0 ) {
		 printf("NULLINDEX\n");
		return;
	}

	if ( (fps = ps_alloc( std_open )) == NULLPS ) {
		 printf( "turbo_index_print: ps_alloc failed\n" );
		return;
	}
	if ( std_setup( fps, stdout ) == NOTOK ) {
		 printf( "turbo_index_print: std_setup failed\n" );
		return;
	}

	ps_print( fps, "*******\n" );
	for ( i = 0; i < turbo_index_num; i++ ) {
		ps_printf( fps, "  Index for attribute (%s)\n",
				   pindex[i].i_attr->oa_ot.ot_name );

		 avl_apply( pindex[i].i_root, print_index_node, (caddr_t) fps,
						  NOTOK, AVL_INORDER );

		ps_printf( fps, "  Soundex index for attribute (%s)\n",
				   pindex[i].i_attr->oa_ot.ot_name );

		 avl_apply( pindex[i].i_sroot, print_soundex_node, (caddr_t) fps,
						  NOTOK, AVL_INORDER );

		ps_print( fps, "  Endof index\n" );
	}
	ps_print( fps, "*******\n" );

	ps_free( fps );
}

int 
print_eis_list (EntryInfo *e)
{
	DN	dnend;

	if ( e == NULLENTRYINFO ) {
		 printf("\tNULL\n");
		return;
	}

	while ( e != NULLENTRYINFO ) {
		for ( dnend = e->ent_dn; dnend->dn_parent != NULLDN;
				dnend = dnend->dn_parent )
			;	/* NULL */

		 printf("\t(%s)\n", dnend->dn_rdn->rdn_av.av_struct);

		e = e->ent_next;
	}
	 printf("(end)\n");
}

int 
print_dn (DN dn)
{
	PS	fps;

	if ( (fps = ps_alloc( std_open )) == NULLPS ) {
		 printf( "turbo_index_print: ps_alloc failed\n" );
		return;
	}
	if ( std_setup( fps, stdout ) == NOTOK ) {
		 printf( "turbo_index_print: std_setup failed\n" );
		return;
	}

	ps_print( fps, "\tDN= " );
	dn_print( fps, dn, EDBOUT );
	ps_print( fps, "\n" );
	 ps_flush( fps );

	ps_free( fps );
}
