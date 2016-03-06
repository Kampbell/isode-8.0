__END_DECLS 
main (int argc, char **argv, char **envp)
{
	int	    len,
			paddr_type;
	char   *prefix,
		   *service,
		   buf[BUFSIZ],
		   buf2[BUFSIZ];
	OSI_ADDR p_addr;

	if (argc < 2) {
		fprintf (stderr,"usage: %s host [service]\n", argv[0]);
		exit (0);
	}
	service = (argc > 2) ? argv[2] : "FTAM";

	/* SUNLink OSI directory lookup */
	mds_lookup (argv[1], service, &p_addr);

	/* SUNLink function to slice out SAP bytes from full address */
	paddr_type = 0;
	if ((len = osi_get_sap (&p_addr, buf, sizeof buf, OSI_NSAP, &paddr_type))
			<= 0) {
		fprintf (stderr, "no entry for %s %s\n", argv[1], service);
		exit (1);
	}

	buf2[explode (buf2, (u_char *) buf, len)]= NULL;
	switch (paddr_type) {
	case AF_NBS:
		prefix = "49";
		break;

	case AF_OSINET:
		prefix = "470004";
		break;

	default:
		prefix = "";
		break;
	}
	printf ("\t\t\t\tNS+%s%s\n\n", prefix, buf2);

	exit (0);
}

/* so we don't have to load libisode.a */

static char nib2hex[0x10] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


static int  explode (a, b, n)
char  *a;
u_char *b;
int    n;
{
	int    i;
	u_char c;

	for (i = 0; i < n; i++) {
		c = *b++;
		*a++ = nib2hex[(c & 0xf0) >> 4];
		*a++ = nib2hex[(c & 0x0f)];
	}
	*a = NULL;

	return (n * 2);
}
