/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

static void declare_singletons(uint32_t kept)
{
    enum bridge_on_destroy on[4] = { BRIDGE_ON_DESTROY_REMOVE,
        BRIDGE_ON_DESTROY_REMOVE, BRIDGE_ON_DESTROY_REMOVE,
        BRIDGE_ON_DESTROY_REMOVE };

    on[kept] = BRIDGE_ON_DESTROY_KEEP;
    BRIDGE_COMPONENT("game:KeptStruct", position_t, BRIDGE_DELETE,
        BRIDGE_SINGLETON, on[1]);
    BRIDGE_DECLARE(BRIDGE_STRUCT, "game:KeptDeclared", position_t,
        BRIDGE_DELETE, BRIDGE_SINGLETON, on[2]);
    BRIDGE_DECLARE_TAG("game:KeptTag", BRIDGE_DELETE, BRIDGE_SINGLETON,
        on[3]);
}

void wrong_keep_component(void)
{
    declare_singletons(1);
}

void wrong_keep_declare(void)
{
    declare_singletons(2);
}

void wrong_keep_tag(void)
{
    declare_singletons(3);
}

void wrong_keep_none(void)
{
    declare_singletons(0);
}
