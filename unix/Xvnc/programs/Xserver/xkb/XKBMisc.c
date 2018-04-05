/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be
used in advertising or publicity pertaining to distribution
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#elif defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "inputstr.h"
#include <X11/keysym.h>
#define	XKBSRV_NEED_FILE_FUNCS
#include <xkbsrv.h>

/***====================================================================***/

#define	CORE_SYM(i)	(i<map_width?core_syms[i]:NoSymbol)
#define	XKB_OFFSET(g,l)	(((g)*groupsWidth)+(l))

int
XkbKeyTypesForCoreSymbols(XkbDescPtr xkb,
                          int map_width,
                          KeySym * core_syms,
                          unsigned int protected,
                          int *types_inout, KeySym * xkb_syms_rtrn)
{
    register int i;
    unsigned int empty;
    int nSyms[XkbNumKbdGroups];
    int nGroups, tmp, groupsWidth;
    BOOL replicated = FALSE;

    /* Section 12.2 of the protocol describes this process in more detail */
    /* Step 1:  find the # of symbols in the core mapping per group */
    groupsWidth = 2;
    for (i = 0; i < XkbNumKbdGroups; i++) {
        if ((protected & (1 << i)) && (types_inout[i] < xkb->map->num_types)) {
            nSyms[i] = xkb->map->types[types_inout[i]].num_levels;
            if (nSyms[i] > groupsWidth)
                groupsWidth = nSyms[i];
        }
        else {
            types_inout[i] = XkbTwoLevelIndex;  /* don't really know, yet */
            nSyms[i] = 2;
        }
    }
    if (nSyms[XkbGroup1Index] < 2)
        nSyms[XkbGroup1Index] = 2;
    if (nSyms[XkbGroup2Index] < 2)
        nSyms[XkbGroup2Index] = 2;
    /* Step 2:  Copy the symbols from the core ordering to XKB ordering */
    /*          symbols in the core are in the order:                   */
    /*          G1L1 G1L2 G2L1 G2L2 [G1L[3-n]] [G2L[3-n]] [G3L*] [G3L*] */
    xkb_syms_rtrn[XKB_OFFSET(XkbGroup1Index, 0)] = CORE_SYM(0);
    xkb_syms_rtrn[XKB_OFFSET(XkbGroup1Index, 1)] = CORE_SYM(1);
    for (i = 2; i < nSyms[XkbGroup1Index]; i++) {
        xkb_syms_rtrn[XKB_OFFSET(XkbGroup1Index, i)] = CORE_SYM(2 + i);
    }
    xkb_syms_rtrn[XKB_OFFSET(XkbGroup2Index, 0)] = CORE_SYM(2);
    xkb_syms_rtrn[XKB_OFFSET(XkbGroup2Index, 1)] = CORE_SYM(3);
    tmp = 2 + (nSyms[XkbGroup1Index] - 2);      /* offset to extra group2 syms */
    for (i = 2; i < nSyms[XkbGroup2Index]; i++) {
        xkb_syms_rtrn[XKB_OFFSET(XkbGroup2Index, i)] = CORE_SYM(tmp + i);
    }

    /* Special case: if only the first group is explicit, and the symbols
     * replicate across all groups, then we have a Section 12.4 replication */
    if ((protected & ~XkbExplicitKeyType1Mask) == 0) {
        int j, width = nSyms[XkbGroup1Index];

        replicated = TRUE;

        /* Check ABAB in ABABCDECDEABCDE */
        if ((width > 0 && CORE_SYM(0) != CORE_SYM(2)) ||
            (width > 1 && CORE_SYM(1) != CORE_SYM(3)))
            replicated = FALSE;

        /* Check CDECDE in ABABCDECDEABCDE */
        for (i = 2; i < width && replicated; i++) {
            if (CORE_SYM(2 + i) != CORE_SYM(i + width))
                replicated = FALSE;
        }

        /* Check ABCDE in ABABCDECDEABCDE */
        for (j = 2; replicated &&
             j < XkbNumKbdGroups && map_width >= width * (j + 1); j++) {
            for (i = 0; i < width && replicated; i++) {
                if (CORE_SYM(((i < 2) ? i : 2 + i)) != CORE_SYM(i + width * j))
                    replicated = FALSE;
            }
        }
    }

    if (replicated) {
        nSyms[XkbGroup2Index] = 0;
        nSyms[XkbGroup3Index] = 0;
        nSyms[XkbGroup4Index] = 0;
        nGroups = 1;
    }
    else {
        tmp = nSyms[XkbGroup1Index] + nSyms[XkbGroup2Index];
        if ((tmp >= map_width) &&
            ((protected & (XkbExplicitKeyType3Mask | XkbExplicitKeyType4Mask))
             == 0)) {
            nSyms[XkbGroup3Index] = 0;
            nSyms[XkbGroup4Index] = 0;
            nGroups = 2;
        }
        else {
            nGroups = 3;
            for (i = 0; i < nSyms[XkbGroup3Index]; i++, tmp++) {
                xkb_syms_rtrn[XKB_OFFSET(XkbGroup3Index, i)] = CORE_SYM(tmp);
            }
            if ((tmp < map_width) || (protected & XkbExplicitKeyType4Mask)) {
                nGroups = 4;
                for (i = 0; i < nSyms[XkbGroup4Index]; i++, tmp++) {
                    xkb_syms_rtrn[XKB_OFFSET(XkbGroup4Index, i)] =
                        CORE_SYM(tmp);
                }
            }
            else {
                nSyms[XkbGroup4Index] = 0;
            }
        }
    }
    /* steps 3&4: alphanumeric expansion,  assign canonical types */
    empty = 0;
    for (i = 0; i < nGroups; i++) {
        KeySym *syms;

        syms = &xkb_syms_rtrn[XKB_OFFSET(i, 0)];
        if ((nSyms[i] > 1) && (syms[1] == NoSymbol) && (syms[0] != NoSymbol)) {
            KeySym upper, lower;

            XkbConvertCase(syms[0], &lower, &upper);
            if (upper != lower) {
                xkb_syms_rtrn[XKB_OFFSET(i, 0)] = lower;
                xkb_syms_rtrn[XKB_OFFSET(i, 1)] = upper;
                if ((protected & (1 << i)) == 0)
                    types_inout[i] = XkbAlphabeticIndex;
            }
            else if ((protected & (1 << i)) == 0) {
                types_inout[i] = XkbOneLevelIndex;
                /*      nSyms[i]=       1; */
            }
        }
        if (((protected & (1 << i)) == 0) &&
            (types_inout[i] == XkbTwoLevelIndex)) {
            if (XkbKSIsKeypad(syms[0]) || XkbKSIsKeypad(syms[1]))
                types_inout[i] = XkbKeypadIndex;
            else {
                KeySym upper, lower;

                XkbConvertCase(syms[0], &lower, &upper);
                if ((syms[0] == lower) && (syms[1] == upper))
                    types_inout[i] = XkbAlphabeticIndex;
            }
        }
        if (syms[0] == NoSymbol) {
            register int n;
            Bool found;

            for (n = 1, found = FALSE; (!found) && (n < nSyms[i]); n++) {
                found = (syms[n] != NoSymbol);
            }
            if (!found)
                empty |= (1 << i);
        }
    }
    /* step 5: squoosh out empty groups */
    if (empty) {
        for (i = nGroups - 1; i >= 0; i--) {
            if (((empty & (1 << i)) == 0) || (protected & (1 << i)))
                break;
            nGroups--;
        }
    }
    if (nGroups < 1)
        return 0;

    /* step 6: replicate group 1 into group two, if necessary */
    if ((nGroups > 1) &&
        ((empty & (XkbGroup1Mask | XkbGroup2Mask)) == XkbGroup2Mask)) {
        if ((protected & (XkbExplicitKeyType1Mask | XkbExplicitKeyType2Mask)) ==
            0) {
            nSyms[XkbGroup2Index] = nSyms[XkbGroup1Index];
            types_inout[XkbGroup2Index] = types_inout[XkbGroup1Index];
            memcpy((char *) &xkb_syms_rtrn[2], (char *) xkb_syms_rtrn,
                   2 * sizeof(KeySym));
        }
        else if (types_inout[XkbGroup1Index] == types_inout[XkbGroup2Index]) {
            memcpy((char *) &xkb_syms_rtrn[nSyms[XkbGroup1Index]],
                   (char *) xkb_syms_rtrn,
                   nSyms[XkbGroup1Index] * sizeof(KeySym));
        }
    }

    /* step 7: check for all groups identical or all width 1
     *
     * Special feature: if group 1 has an explicit type and all other groups
     * have canonical types with same symbols, we assume it's info lost from
     * the core replication.
     */
    if (nGroups > 1) {
        Bool sameType, allOneLevel, canonical = TRUE;

        allOneLevel = (xkb->map->types[types_inout[0]].num_levels == 1);
        for (i = 1, sameType = TRUE; (allOneLevel || sameType) && (i < nGroups);
             i++) {
            sameType = (sameType &&
                        (types_inout[i] == types_inout[XkbGroup1Index]));
            if (allOneLevel)
                allOneLevel = (xkb->map->types[types_inout[i]].num_levels == 1);
            if (types_inout[i] > XkbLastRequiredType)
                canonical = FALSE;
        }
        if (((sameType) || canonical) &&
            (!(protected &
               (XkbExplicitKeyTypesMask & ~XkbExplicitKeyType1Mask)))) {
            register int s;
            Bool identical;

            for (i = 1, identical = TRUE; identical && (i < nGroups); i++) {
                KeySym *syms;

                if (nSyms[i] != nSyms[XkbGroup1Index])
                    identical = FALSE;
                syms = &xkb_syms_rtrn[XKB_OFFSET(i, 0)];
                for (s = 0; identical && (s < nSyms[i]); s++) {
                    if (syms[s] != xkb_syms_rtrn[s])
                        identical = FALSE;
                }
            }
            if (identical)
                nGroups = 1;
        }
        if (allOneLevel && (nGroups > 1)) {
            KeySym *syms;

            syms = &xkb_syms_rtrn[nSyms[XkbGroup1Index]];
            nSyms[XkbGroup1Index] = 1;
            for (i = 1; i < nGroups; i++) {
                xkb_syms_rtrn[i] = syms[0];
                syms += nSyms[i];
                nSyms[i] = 1;
            }
        }
    }
    return nGroups;
}

static XkbSymInterpretPtr
_XkbFindMatchingInterp(XkbDescPtr xkb,
                       KeySym sym, unsigned int real_mods, unsigned int level)
{
    register unsigned i;
    XkbSymInterpretPtr interp, rtrn;
    CARD8 mods;

    rtrn = NULL;
    interp = xkb->compat->sym_interpret;
    for (i = 0; i < xkb->compat->num_si; i++, interp++) {
        if ((interp->sym == NoSymbol) || (sym == interp->sym)) {
            int match;

            if ((level == 0) || ((interp->match & XkbSI_LevelOneOnly) == 0))
                mods = real_mods;
            else
                mods = 0;
            switch (interp->match & XkbSI_OpMask) {
            case XkbSI_NoneOf:
                match = ((interp->mods & mods) == 0);
                break;
            case XkbSI_AnyOfOrNone:
                match = ((mods == 0) || ((interp->mods & mods) != 0));
                break;
            case XkbSI_AnyOf:
                match = ((interp->mods & mods) != 0);
                break;
            case XkbSI_AllOf:
                match = ((interp->mods & mods) == interp->mods);
                break;
            case XkbSI_Exactly:
                match = (interp->mods == mods);
                break;
            default:
                match = 0;
                break;
            }
            if (match) {
                if (interp->sym != NoSymbol) {
                    return interp;
                }
                else if (rtrn == NULL) {
                    rtrn = interp;
                }
            }
        }
    }
    return rtrn;
}

static void
_XkbAddKeyChange(KeyCode *pFirst, unsigned char *pNum, KeyCode newKey)
{
    KeyCode last;

    last = (*pFirst) + (*pNum);
    if (newKey < *pFirst) {
        *pFirst = newKey;
        *pNum = (last - newKey) + 1;
    }
    else if (newKey > last) {
        *pNum = (last - *pFirst) + 1;
    }
    return;
}

static void
_XkbSetActionKeyMods(XkbDescPtr xkb, XkbAction *act, unsigned mods)
{
    unsigned tmp;

    switch (act->type) {
    case XkbSA_SetMods:
    case XkbSA_LatchMods:
    case XkbSA_LockMods:
        if (act->mods.flags & XkbSA_UseModMapMods)
            act->mods.real_mods = act->mods.mask = mods;
        if ((tmp = XkbModActionVMods(&act->mods)) != 0) {
            XkbVirtualModsToReal(xkb, tmp, &tmp);
            act->mods.mask |= tmp;
        }
        break;
    case XkbSA_ISOLock:
        if (act->iso.flags & XkbSA_UseModMapMods)
            act->iso.real_mods = act->iso.mask = mods;
        if ((tmp = XkbModActionVMods(&act->iso)) != 0) {
            XkbVirtualModsToReal(xkb, tmp, &tmp);
            act->iso.mask |= tmp;
        }
        break;
    }
    return;
}

#define	IBUF_SIZE	8

Bool
XkbApplyCompatMapToKey(XkbDescPtr xkb, KeyCode key, XkbChangesPtr changes)
{
    KeySym *syms;
    unsigned char explicit, mods;
    XkbSymInterpretPtr *interps, ibuf[IBUF_SIZE];
    int n, nSyms, found;
    unsigned changed, tmp;

    if ((!xkb) || (!xkb->map) || (!xkb->map->key_sym_map) ||
        (!xkb->compat) || (!xkb->compat->sym_interpret) ||
        (key < xkb->min_key_code) || (key > xkb->max_key_code)) {
        return FALSE;
    }
    if (((!xkb->server) || (!xkb->server->key_acts)) &&
        (XkbAllocServerMap(xkb, XkbAllServerInfoMask, 0) != Success)) {
        return FALSE;
    }
    changed = 0;                /* keeps track of what has changed in _this_ call */
    explicit = xkb->server->explicit[key];
    if (explicit & XkbExplicitInterpretMask)    /* nothing to do */
        return TRUE;
    mods = (xkb->map->modmap ? xkb->map->modmap[key] : 0);
    nSyms = XkbKeyNumSyms(xkb, key);
    syms = XkbKeySymsPtr(xkb, key);
    if (nSyms > IBUF_SIZE) {
        interps = calloc(nSyms, sizeof(XkbSymInterpretPtr));
        if (interps == NULL) {
            interps = ibuf;
            nSyms = IBUF_SIZE;
        }
    }
    else {
        interps = ibuf;
    }
    found = 0;
    for (n = 0; n < nSyms; n++) {
        unsigned level = (n % XkbKeyGroupsWidth(xkb, key));

        interps[n] = NULL;
        if (syms[n] != NoSymbol) {
            interps[n] = _XkbFindMatchingInterp(xkb, syms[n], mods, level);
            if (interps[n] && interps[n]->act.type != XkbSA_NoAction)
                found++;
            else
                interps[n] = NULL;
        }
    }
    /* 1/28/96 (ef) -- XXX! WORKING HERE */
    if (!found) {
        if (xkb->server->key_acts[key] != 0) {
            xkb->server->key_acts[key] = 0;
            changed |= XkbKeyActionsMask;
        }
    }
    else {
        XkbAction *pActs;
        unsigned int new_vmodmask;

        changed |= XkbKeyActionsMask;
        pActs = XkbResizeKeyActions(xkb, key, nSyms);
        if (!pActs) {
            if (nSyms > IBUF_SIZE)
                free(interps);
            return FALSE;
        }
        new_vmodmask = 0;
        for (n = 0; n < nSyms; n++) {
            if (interps[n]) {
                unsigned effMods;

                pActs[n] = *((XkbAction *) &interps[n]->act);
                if ((n == 0) || ((interps[n]->match & XkbSI_LevelOneOnly) == 0)) {
                    effMods = mods;
                    if (interps[n]->virtual_mod != XkbNoModifier)
                        new_vmodmask |= (1 << interps[n]->virtual_mod);
                }
                else
                    effMods = 0;
                _XkbSetActionKeyMods(xkb, &pActs[n], effMods);
            }
            else
                pActs[n].type = XkbSA_NoAction;
        }
        if (((explicit & XkbExplicitVModMapMask) == 0) &&
            (xkb->server->vmodmap[key] != new_vmodmask)) {
            changed |= XkbVirtualModMapMask;
            xkb->server->vmodmap[key] = new_vmodmask;
        }
        if (interps[0]) {
            if ((interps[0]->flags & XkbSI_LockingKey) &&
                ((explicit & XkbExplicitBehaviorMask) == 0)) {
                xkb->server->behaviors[key].type = XkbKB_Lock;
                changed |= XkbKeyBehaviorsMask;
            }
            if (((explicit & XkbExplicitAutoRepeatMask) == 0) && (xkb->ctrls)) {
                CARD8 old;

                old = BitIsOn(xkb->ctrls->per_key_repeat, key);
                if (interps[0]->flags & XkbSI_AutoRepeat)
                    SetBit(xkb->ctrls->per_key_repeat, key);
                else
                    ClearBit(xkb->ctrls->per_key_repeat, key);
                if (changes && old != BitIsOn(xkb->ctrls->per_key_repeat, key))
                    changes->ctrls.changed_ctrls |= XkbPerKeyRepeatMask;
            }
        }
    }
    if ((!found) || (interps[0] == NULL)) {
        if (((explicit & XkbExplicitAutoRepeatMask) == 0) && (xkb->ctrls)) {
            CARD8 old;

            old = BitIsOn(xkb->ctrls->per_key_repeat, key);
            SetBit(xkb->ctrls->per_key_repeat, key);
            if (changes && (old != BitIsOn(xkb->ctrls->per_key_repeat, key)))
                changes->ctrls.changed_ctrls |= XkbPerKeyRepeatMask;
        }
        if (((explicit & XkbExplicitBehaviorMask) == 0) &&
            (xkb->server->behaviors[key].type == XkbKB_Lock)) {
            xkb->server->behaviors[key].type = XkbKB_Default;
            changed |= XkbKeyBehaviorsMask;
        }
    }
    if (changes) {
        XkbMapChangesPtr mc;

        mc = &changes->map;
        tmp = (changed & mc->changed);
        if (tmp & XkbKeyActionsMask)
            _XkbAddKeyChange(&mc->first_key_act, &mc->num_key_acts, key);
        else if (changed & XkbKeyActionsMask) {
            mc->changed |= XkbKeyActionsMask;
            mc->first_key_act = key;
            mc->num_key_acts = 1;
        }
        if (tmp & XkbKeyBehaviorsMask) {
            _XkbAddKeyChange(&mc->first_key_behavior, &mc->num_key_behaviors,
                             key);
        }
        else if (changed & XkbKeyBehaviorsMask) {
            mc->changed |= XkbKeyBehaviorsMask;
            mc->first_key_behavior = key;
            mc->num_key_behaviors = 1;
        }
        if (tmp & XkbVirtualModMapMask)
            _XkbAddKeyChange(&mc->first_vmodmap_key, &mc->num_vmodmap_keys,
                             key);
        else if (changed & XkbVirtualModMapMask) {
            mc->changed |= XkbVirtualModMapMask;
            mc->first_vmodmap_key = key;
            mc->num_vmodmap_keys = 1;
        }
        mc->changed |= changed;
    }
    if (interps != ibuf)
        free(interps);
    return TRUE;
}

Status
XkbChangeTypesOfKey(XkbDescPtr xkb,
                    int key,
                    int nGroups,
                    unsigned groups, int *newTypesIn, XkbMapChangesPtr changes)
{
    XkbKeyTypePtr pOldType, pNewType;
    register int i;
    int width, nOldGroups, oldWidth, newTypes[XkbNumKbdGroups];

    if ((!xkb) || (!XkbKeycodeInRange(xkb, key)) || (!xkb->map) ||
        (!xkb->map->types) || (!newTypesIn) ||
        ((groups & XkbAllGroupsMask) == 0) || (nGroups > XkbNumKbdGroups)) {
        return BadMatch;
    }
    if (nGroups == 0) {
        for (i = 0; i < XkbNumKbdGroups; i++) {
            xkb->map->key_sym_map[key].kt_index[i] = XkbOneLevelIndex;
        }
        i = xkb->map->key_sym_map[key].group_info;
        i = XkbSetNumGroups(i, 0);
        xkb->map->key_sym_map[key].group_info = i;
        XkbResizeKeySyms(xkb, key, 0);
        return Success;
    }

    nOldGroups = XkbKeyNumGroups(xkb, key);
    oldWidth = XkbKeyGroupsWidth(xkb, key);
    for (width = i = 0; i < nGroups; i++) {
        if (groups & (1 << i))
            newTypes[i] = newTypesIn[i];
        else if (i < nOldGroups)
            newTypes[i] = XkbKeyKeyTypeIndex(xkb, key, i);
        else if (nOldGroups > 0)
            newTypes[i] = XkbKeyKeyTypeIndex(xkb, key, XkbGroup1Index);
        else
            newTypes[i] = XkbTwoLevelIndex;
        if (newTypes[i] > xkb->map->num_types)
            return BadMatch;
        pNewType = &xkb->map->types[newTypes[i]];
        if (pNewType->num_levels > width)
            width = pNewType->num_levels;
    }
    if ((xkb->ctrls) && (nGroups > xkb->ctrls->num_groups))
        xkb->ctrls->num_groups = nGroups;
    if ((width != oldWidth) || (nGroups != nOldGroups)) {
        KeySym oldSyms[XkbMaxSymsPerKey], *pSyms;
        int nCopy;

        if (nOldGroups == 0) {
            pSyms = XkbResizeKeySyms(xkb, key, width * nGroups);
            if (pSyms != NULL) {
                i = xkb->map->key_sym_map[key].group_info;
                i = XkbSetNumGroups(i, nGroups);
                xkb->map->key_sym_map[key].group_info = i;
                xkb->map->key_sym_map[key].width = width;
                for (i = 0; i < nGroups; i++) {
                    xkb->map->key_sym_map[key].kt_index[i] = newTypes[i];
                }
                return Success;
            }
            return BadAlloc;
        }
        pSyms = XkbKeySymsPtr(xkb, key);
        memcpy(oldSyms, pSyms, XkbKeyNumSyms(xkb, key) * sizeof(KeySym));
        pSyms = XkbResizeKeySyms(xkb, key, width * nGroups);
        if (pSyms == NULL)
            return BadAlloc;
        memset(pSyms, 0, width * nGroups * sizeof(KeySym));
        for (i = 0; (i < nGroups) && (i < nOldGroups); i++) {
            pOldType = XkbKeyKeyType(xkb, key, i);
            pNewType = &xkb->map->types[newTypes[i]];
            if (pNewType->num_levels > pOldType->num_levels)
                nCopy = pOldType->num_levels;
            else
                nCopy = pNewType->num_levels;
            memcpy(&pSyms[i * width], &oldSyms[i * oldWidth],
                   nCopy * sizeof(KeySym));
        }
        if (XkbKeyHasActions(xkb, key)) {
            XkbAction oldActs[XkbMaxSymsPerKey], *pActs;

            pActs = XkbKeyActionsPtr(xkb, key);
            memcpy(oldActs, pActs, XkbKeyNumSyms(xkb, key) * sizeof(XkbAction));
            pActs = XkbResizeKeyActions(xkb, key, width * nGroups);
            if (pActs == NULL)
                return BadAlloc;
            memset(pActs, 0, width * nGroups * sizeof(XkbAction));
            for (i = 0; (i < nGroups) && (i < nOldGroups); i++) {
                pOldType = XkbKeyKeyType(xkb, key, i);
                pNewType = &xkb->map->types[newTypes[i]];
                if (pNewType->num_levels > pOldType->num_levels)
                    nCopy = pOldType->num_levels;
                else
                    nCopy = pNewType->num_levels;
                memcpy(&pActs[i * width], &oldActs[i * oldWidth],
                       nCopy * sizeof(XkbAction));
            }
        }
        i = xkb->map->key_sym_map[key].group_info;
        i = XkbSetNumGroups(i, nGroups);
        xkb->map->key_sym_map[key].group_info = i;
        xkb->map->key_sym_map[key].width = width;
    }
    width = 0;
    for (i = 0; i < nGroups; i++) {
        xkb->map->key_sym_map[key].kt_index[i] = newTypes[i];
        if (xkb->map->types[newTypes[i]].num_levels > width)
            width = xkb->map->types[newTypes[i]].num_levels;
    }
    xkb->map->key_sym_map[key].width = width;
    if (changes != NULL) {
        if (changes->changed & XkbKeySymsMask) {
            _XkbAddKeyChange(&changes->first_key_sym, &changes->num_key_syms,
                             key);
        }
        else {
            changes->changed |= XkbKeySymsMask;
            changes->first_key_sym = key;
            changes->num_key_syms = 1;
        }
    }
    return Success;
}

/***====================================================================***/

Bool
XkbVirtualModsToReal(XkbDescPtr xkb, unsigned virtual_mask, unsigned *mask_rtrn)
{
    register int i, bit;
    register unsigned mask;

    if (xkb == NULL)
        return FALSE;
    if (virtual_mask == 0) {
        *mask_rtrn = 0;
        return TRUE;
    }
    if (xkb->server == NULL)
        return FALSE;
    for (i = mask = 0, bit = 1; i < XkbNumVirtualMods; i++, bit <<= 1) {
        if (virtual_mask & bit)
            mask |= xkb->server->vmods[i];
    }
    *mask_rtrn = mask;
    return TRUE;
}

/***====================================================================***/

static Bool
XkbUpdateActionVirtualMods(XkbDescPtr xkb, XkbAction *act, unsigned changed)
{
    unsigned int tmp;

    switch (act->type) {
    case XkbSA_SetMods:
    case XkbSA_LatchMods:
    case XkbSA_LockMods:
        if (((tmp = XkbModActionVMods(&act->mods)) & changed) != 0) {
            XkbVirtualModsToReal(xkb, tmp, &tmp);
            act->mods.mask = act->mods.real_mods;
            act->mods.mask |= tmp;
            return TRUE;
        }
        break;
    case XkbSA_ISOLock:
        if ((((tmp = XkbModActionVMods(&act->iso)) != 0) & changed) != 0) {
            XkbVirtualModsToReal(xkb, tmp, &tmp);
            act->iso.mask = act->iso.real_mods;
            act->iso.mask |= tmp;
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static void
XkbUpdateKeyTypeVirtualMods(XkbDescPtr xkb,
                            XkbKeyTypePtr type,
                            unsigned int changed, XkbChangesPtr changes)
{
    register unsigned int i;
    unsigned int mask = 0;

    XkbVirtualModsToReal(xkb, type->mods.vmods, &mask);
    type->mods.mask = type->mods.real_mods | mask;
    if ((type->map_count > 0) && (type->mods.vmods != 0)) {
        XkbKTMapEntryPtr entry;

        for (i = 0, entry = type->map; i < type->map_count; i++, entry++) {
            if (entry->mods.vmods != 0) {
                XkbVirtualModsToReal(xkb, entry->mods.vmods, &mask);
                entry->mods.mask = entry->mods.real_mods | mask;
                /* entry is active if vmods are bound */
                entry->active = (mask != 0);
            }
            else
                entry->active = 1;
        }
    }
    if (changes) {
        int type_ndx;

        type_ndx = type - xkb->map->types;
        if ((type_ndx < 0) || (type_ndx > xkb->map->num_types))
            return;
        if (changes->map.changed & XkbKeyTypesMask) {
            int last;

            last = changes->map.first_type + changes->map.num_types - 1;
            if (type_ndx < changes->map.first_type) {
                changes->map.first_type = type_ndx;
                changes->map.num_types = (last - type_ndx) + 1;
            }
            else if (type_ndx > last) {
                changes->map.num_types =
                    (type_ndx - changes->map.first_type) + 1;
            }
        }
        else {
            changes->map.changed |= XkbKeyTypesMask;
            changes->map.first_type = type_ndx;
            changes->map.num_types = 1;
        }
    }
    return;
}

Bool
XkbApplyVirtualModChanges(XkbDescPtr xkb, unsigned changed,
                          XkbChangesPtr changes)
{
    register int i;
    unsigned int checkState = 0;

    if ((!xkb) || (!xkb->map) || (changed == 0))
        return FALSE;
    for (i = 0; i < xkb->map->num_types; i++) {
        if (xkb->map->types[i].mods.vmods & changed)
            XkbUpdateKeyTypeVirtualMods(xkb, &xkb->map->types[i], changed,
                                        changes);
    }
    if (changed & xkb->ctrls->internal.vmods) {
        unsigned int newMask = 0;

        XkbVirtualModsToReal(xkb, xkb->ctrls->internal.vmods, &newMask);
        newMask |= xkb->ctrls->internal.real_mods;
        if (xkb->ctrls->internal.mask != newMask) {
            xkb->ctrls->internal.mask = newMask;
            if (changes) {
                changes->ctrls.changed_ctrls |= XkbInternalModsMask;
                checkState = TRUE;
            }
        }
    }
    if (changed & xkb->ctrls->ignore_lock.vmods) {
        unsigned int newMask = 0;

        XkbVirtualModsToReal(xkb, xkb->ctrls->ignore_lock.vmods, &newMask);
        newMask |= xkb->ctrls->ignore_lock.real_mods;
        if (xkb->ctrls->ignore_lock.mask != newMask) {
            xkb->ctrls->ignore_lock.mask = newMask;
            if (changes) {
                changes->ctrls.changed_ctrls |= XkbIgnoreLockModsMask;
                checkState = TRUE;
            }
        }
    }
    if (xkb->indicators != NULL) {
        XkbIndicatorMapPtr map;

        map = &xkb->indicators->maps[0];
        for (i = 0; i < XkbNumIndicators; i++, map++) {
            if (map->mods.vmods & changed) {
                unsigned int newMask = 0;

                XkbVirtualModsToReal(xkb, map->mods.vmods, &newMask);
                newMask |= map->mods.real_mods;
                if (newMask != map->mods.mask) {
                    map->mods.mask = newMask;
                    if (changes) {
                        changes->indicators.map_changes |= (1 << i);
                        checkState = TRUE;
                    }
                }
            }
        }
    }
    if (xkb->compat != NULL) {
        XkbCompatMapPtr compat;

        compat = xkb->compat;
        for (i = 0; i < XkbNumKbdGroups; i++) {
            unsigned int newMask = 0;

            XkbVirtualModsToReal(xkb, compat->groups[i].vmods, &newMask);
            newMask |= compat->groups[i].real_mods;
            if (compat->groups[i].mask != newMask) {
                compat->groups[i].mask = newMask;
                if (changes) {
                    changes->compat.changed_groups |= (1 << i);
                    checkState = TRUE;
                }
            }
        }
    }
    if (xkb->map && xkb->server) {
        int highChange = 0, lowChange = -1;

        for (i = xkb->min_key_code; i <= xkb->max_key_code; i++) {
            if (XkbKeyHasActions(xkb, i)) {
                register XkbAction *pAct;
                register int n;

                pAct = XkbKeyActionsPtr(xkb, i);
                for (n = XkbKeyNumActions(xkb, i); n > 0; n--, pAct++) {
                    if ((pAct->type != XkbSA_NoAction) &&
                        XkbUpdateActionVirtualMods(xkb, pAct, changed)) {
                        if (lowChange < 0)
                            lowChange = i;
                        highChange = i;
                    }
                }
            }
        }
        if (changes && (lowChange > 0)) {       /* something changed */
            if (changes->map.changed & XkbKeyActionsMask) {
                int last;

                if (changes->map.first_key_act < lowChange)
                    lowChange = changes->map.first_key_act;
                last =
                    changes->map.first_key_act + changes->map.num_key_acts - 1;
                if (last > highChange)
                    highChange = last;
            }
            changes->map.changed |= XkbKeyActionsMask;
            changes->map.first_key_act = lowChange;
            changes->map.num_key_acts = (highChange - lowChange) + 1;
        }
    }
    return checkState;
}
