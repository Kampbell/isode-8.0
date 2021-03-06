-- **************************************************************
--                                                              *
--  HULA project - connectionless ISODE                         *
--                                             			*
--  module:  	acs.py                                          *
--  author:   	Bill Haggerty                                   *
--  date:     	4/89                                            *
--                                                              *
--  This is an abstract syntax module for ACSE PCI.             *
--  It combines connection-oriented PCI with the connectionless *
--  PCI for A-UNIT-DATA.                                        *
--                                  				*
-- **************************************************************
--								*
--			     NOTICE		   		*
--								*
--    Use of this module is subject to the restrictions of the  *
--    ISODE license agreement.					*
--								*    
-- **************************************************************

-- modifies form acs.py, no changes made to the acs.py header

-- acs.py - AcSAP definitions
--	lifted directly from ISO8650
    
-- $Header: /f/iso/acsap/RCS/acs.py,v 5.0 88/07/21 14:21:02 mrose Rel $
--
--
-- $Log$
-- 

--* ACSE-1 *-- ACS DEFINITIONS ::=

%{
#ifndef	lint
static char *rcsid = "$Header: /f/iso/acsap/RCS/acs.py,v 5.0 88/07/21 14:21:02 mrose Rel $";
#endif
%}



BEGIN

-- ACSE-1 refers to ACSE version 1


-- HULA begins cheating
-- HULA is cheating by sticking A-UNIT-DATA here in this module
-- even though A-UNIT-DATA is a separate abstract syntax, 
-- to prevent dozens of duplicate encode/decode routines 
--
-- ISO-8650/AD2-ACSE-1 refers to ISO 8650/AD2 Version 1


AUDT-apdu ::=
    [APPLICATION 0]
        IMPLICIT SEQUENCE {
            protocol-version[0]
                IMPLICIT BIT STRING {
                    version1(0)
                }
                DEFAULT version1,

            application-context-name[1]
                Application-context-name,

            called-AP-title[2]
                AP-title
                OPTIONAL,

            called-AE-qualifier[3]
                AE-qualifier
                OPTIONAL,

            called-AP-invocation-id[4]
                AP-invocation-id
                OPTIONAL,

            called-AE-invocation-id[5]
                AE-invocation-id
                OPTIONAL,

            calling-AP-title[6]
                AP-title
                OPTIONAL,

            calling-AE-qualifier[7]
                AE-qualifier
                OPTIONAL,

            calling-AP-invocation-id[8]
                AP-invocation-id
                OPTIONAL,

            calling-AE-invocation-id[9]
                AE-invocation-id
                OPTIONAL,

            implementation-information[29]
                IMPLICIT Implementation-data
                OPTIONAL,

            user-information[30]
                IMPLICIT Association-information
        }

-- used Association-information, instead of direct SEQUENCE OF EXTERNAL,
-- because need to use code from AcAssocRequest based on that structure

-- HULA ends cheating


ACSE-apdu ::=
        CHOICE {
            aarq
                AARQ-apdu,

            aare
                AARE-apdu,

            rlrq
                RLRQ-apdu,

            rlre
                RLRE-apdu,

            abrt
                ABRT-apdu
        }


AARQ-apdu ::=
    [APPLICATION 0]
        IMPLICIT SEQUENCE {
            protocol-version[0]
                IMPLICIT BIT STRING {
                    version1(0)
                }
                DEFAULT version1,

            application-context-name[1]
                Application-context-name,

            called-AP-title[2]
                AP-title
                OPTIONAL,

            called-AE-qualifier[3]
                AE-qualifier
                OPTIONAL,

            called-AP-invocation-id[4]
                AP-invocation-id
                OPTIONAL,

            called-AE-invocation-id[5]
                AE-invocation-id
                OPTIONAL,

            calling-AP-title[6]
                AP-title
                OPTIONAL,

            calling-AE-qualifier[7]
                AE-qualifier
                OPTIONAL,

            calling-AP-invocation-id[8]
                AP-invocation-id
                OPTIONAL,

            calling-AE-invocation-id[9]
                AE-invocation-id
                OPTIONAL,

            implementation-information[29]
                IMPLICIT Implementation-data
                OPTIONAL,

            user-information[30]
                IMPLICIT Association-information
                OPTIONAL
        }


AARE-apdu ::=
    [APPLICATION 1]
        IMPLICIT SEQUENCE {
            protocol-version[0]
                IMPLICIT BIT STRING {
                    version1(0)
                }
                DEFAULT version1,

            application-context-name[1]
                Application-context-name,

            result[2]
                --* Associate-result, *--
	        INTEGER {			--* *--
	            accepted(0),		--* *--
	            rejected-permanent(1),	--* *--
	            rejected-transient(2)	--* *--
	        },				--* *--

            result-source-diagnostic[3]
                Associate-source-diagnostic,

            responding-AP-title[4]
                AP-title
                OPTIONAL,

            responding-AE-qualifier[5]
                AE-qualifier
                OPTIONAL,

            responding-AP-invocation-id[6]
                AP-invocation-id
                OPTIONAL,

            responding-AE-invocation-id[7]
                AE-invocation-id
                OPTIONAL,

            implementation-information[29]
                IMPLICIT Implementation-data
                OPTIONAL,

            user-information[30]
                IMPLICIT Association-information
                OPTIONAL
        }


RLRQ-apdu ::=
    [APPLICATION 2]
        IMPLICIT SEQUENCE {
            reason[0]
                IMPLICIT --* Release-request-reason *--
	        INTEGER {			--* *--
	            normal(0),			--* *--
	            urgent(1),			--* *--
	            user-defined(30)		--* *--
	        }				--* *--
                --* OPTIONAL *-- DEFAULT 0,

            user-information[30]
                IMPLICIT Association-information
                OPTIONAL
        }


RLRE-apdu ::=
    [APPLICATION 3]
        IMPLICIT SEQUENCE {
            reason[0]
                IMPLICIT --* Release-response-reason *--
	        INTEGER {			--* *--
	            normal(0),			--* *--
	            not-finished(1),		--* *--
	            user-defined(30)		--* *--
	        }				--* *--
                --* OPTIONAL *-- DEFAULT 0,

            user-information[30]
                IMPLICIT Association-information
                OPTIONAL
        }


ABRT-apdu ::=
    [APPLICATION 4]
        IMPLICIT SEQUENCE {
            abort-source[0]
	     --* IMPLICIT ABRT-source, *--
	         IMPLICIT INTEGER {		--* *--
	             acse-service-user(0),	--* *--
	             acse-service-provider(1)	--* *--
	         },				--* *--


            user-information[30]
                IMPLICIT Association-information
                OPTIONAL
        }

ABRT-source ::=
        INTEGER {
            acse-service-user(0),
            acse-service-provider(1)
        }


Application-context-name ::=
        OBJECT IDENTIFIER


AP-title ::=
        ANY
    -- The exact definition and values used for AP-title
    -- should be chosen taking into account the ongoing
    -- work in the areas of naming, the Directory, and the
    -- Registration Authority procedures for AP-titles,
    -- AE-titles and AE-qualifiers.

AE-qualifier ::=
        ANY
    -- The exact definition and values used for AE-qualifier
    -- should be chosen taking into account the ongoing
    -- work in the areas of naming, the Directory, and the
    -- Registration Authority procedures for AP-titles,
    -- AE-titles and AE-qualifiers.

-- As defined in ISO 7498-3, an application-entity title is composed of
-- an application-process title and an application-entity qualifier.
-- The ACSE protocol provides for the transfer of an application-entity
-- title value by the transfer of its component values.  However, the
-- following data type is provided by reference for other International
-- Standards that require a single syntactic structure for AE-titles.

AE-title ::=
        SEQUENCE {
	    title --* *--
		AP-title,

	    qualifier --* *--
		AE-qualifier
	}

AE-invocation-id ::=
        INTEGER

AP-invocation-id ::=
        INTEGER


Associate-result ::=
        INTEGER {
            accepted(0),
            rejected-permanent(1),
            rejected-transient(2)
        }

Associate-source-diagnostic ::=
        CHOICE {
            acse-service-user[1]
                INTEGER {
                    null(0),
                    no-reason-given(1),
                    application-context-name-not-supported(2),
                    calling-AP-title-not-recognized(3),
                    calling-AP-invocation-identifier-not-recognized(4),
                    calling-AE-qualifier-not-recognized(5),
                    calling-AE-invocation-id-not-recognized(6),
                    called-AP-title-not-recognized(7),
                    called-AP-invocation-identifier-not-recognized(8),
                    called-AE-qualifier-not-recognized(9),
                    called-AE-invocation-id-not-recognized(10)
                },

            acse-service-provider[2]
                INTEGER {
                    null(0),
                    no-reason-given(1),
                    no-common-acse-version(2)
                }
        }

Association-information ::=
        SEQUENCE OF
            EXTERNAL

Implementation-data ::=
        GraphicString

Release-request-reason ::=
        INTEGER {
            normal(0),
            urgent(1),
            user-defined(30)
        }

Release-response-reason ::=
        INTEGER {
            normal(0),
            not-finished(1),
            user-defined(30)
        }

END
