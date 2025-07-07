// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// ATOM IMPLEMENTATION
// KEEPER REQUISITES: core
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_BK_TM_ATOM_C
#define SRC_BK_TM_ATOM_C "bk/tm/atom.c"


#include "core.c"


pub btypeid_t tm_init_atom(BK_TM *tm, btypeid_t btypeid, btypeid_t implicitid, bool explicit) {
    // answers an initialised atom btypeid reserving if necessary
    if (!btypeid || btypeid >= tm->next_btypeId) return B_NAT;
    if (btypeid == B_NEW) {
        btypeid = _update_type_summary(tm, tm->next_btypeId, 0, 0, 0);
    } else {
        // check we are referring to an uninitialised atom
        if (TM_BMT_ID(tm->btsummary_by_btypeid[btypeid]) != bmterr) return B_NAT;
    }

    // initialise
    tm->btsummary_by_btypeid[btypeid] |= bmtatm | (explicit ? TM_IS_EXPLICIT_MASK : 0);
    if (implicitid) tm->implicitid_by_spaceid[btypeid] = implicitid;
    return btypeid;
}

pub btypeid_t tm_check_atom(BK_TM *tm, btypeid_t btypeid, btypeid_t implicitid, bool explicit, btypeid_t spaceid) {
    // An existing atom is being defined a second time check that the attributes don't conflict
    //      a) if current.space is already set then space may be the same or missing
    //      b) if current.implicitly is already set then implicitly may be the same or missing
    //      c) if current.explicit is true then explicit may be True or missing

    return btypeid;
}


#endif  // SRC_BK_TM_ATOM_C