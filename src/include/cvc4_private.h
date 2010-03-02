/*********************                                                        */
/** cvc4_private.h
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.
 **
 ** #inclusion of this file marks a header as private and generates a
 ** warning when the file is included improperly.
 **/

#ifndef __CVC4_PRIVATE_H
#define __CVC4_PRIVATE_H

#if ! (defined(__BUILDING_CVC4LIB) || defined(__BUILDING_CVC4LIB_UNIT_TEST))
#  warning A private CVC4 header was included when not building the library or private unit test code.
#endif /* ! (__BUILDING_CVC4LIB || __BUILDING_CVC4LIB_UNIT_TEST) */

#endif /* __CVC4_PRIVATE_H */