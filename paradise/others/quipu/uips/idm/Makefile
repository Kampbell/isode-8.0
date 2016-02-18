LIBES   =       $(TOPDIR)libdsap.a $(TOPDIR)libisode.a
LLIBS   =       $(TOPDIR)llib-ldsap $(TOPDIR)llib-lisode

###############################################################################
# FILES
###############################################################################

CFILES	=	dm.c bind.c init.c util.c filter.c people.c ou.c org.c \
                country.c cname.c video.c list.c atts.c pager.c mapatt.c  \
		ids.c posdit.c help.c rawinput.c mapphone.c add.c \
		delete.c modify.c read.c listdm.c misc.c \
		peopleoc.c peoplerl.c peoplerm.c peoplecp.c

OFILES	=	dm.o bind.o init.o util.o filter.o people.o ou.o org.o \
                country.o cname.o video.o list.o atts.o pager.o mapatt.o \
		ids.o posdit.o help.o rawinput.o mapphone.o add.o \
		delete.o modify.o read.o listdm.o misc.o  \
		peopleoc.o peoplerl.o peoplerm.o peoplecp.o

HFILES  =       bind.h config.h destrings.h namelist.h util.h \
                cnamelist.h demanifest.h filter.h types.h mapatts.h \
		messages.h mapphone.h extmessages.h

all:	idm
inst-all: inst-idm help
install: inst-all clean

inst-idm:	$(BINDIR)idm

$(BINDIR)idm:	xdm
		-cp $@ zxdm
		-rm -f $@
		cp xdm $@
		-@ls -gls $@
		-@echo ""

idm:		xdm		

xdm:	$(OFILES) $(LIBES)
	$(LDCC) $(LDFLAGS) -o $@ $(OFILES) \
	        $(LIBDSAP) $(LIBISODE) $(LSOCKET) -ltermcap
	
help:		$(ETCDIR)idm/dmtailor
		-cp -r etc/* $(ETCDIR)idm
		-@ls -gls $(ETCDIR)idm

$(ETCDIR)idm/dmtailor: dmtailor
		-mkdir $(ETCDIR)idm
		if [ -s $@ ]; \
		    then exit 0; \
		    else cp dmtailor $@; ls -gls $@; \
		fi
		if ln $(ETCDIR)idm/dmtailor $(ETCDIR)idm/dmtailor.old; \
		    then rm -f $@ ; cp dmtailor $@; ls -gls $@; \
		    else exit 0; \
		fi

dmtailor:



##############################################################
# clean
##############################################################

clean:;         rm -f *.ph *.o *.a a.out _* x* z* *.orig core

grind:;

true:;
		
#depend:;	makedepend $(CFILES)

# DO NOT DELETE THIS LINE -- make depend depends on it.

