#include "storage.h"

#include "building/building.h"
#include "building/rotation.h"
#include "city/resource.h"

#include "io/io_buffer.h"
#include "window/city.h"
#include "window/popup_dialog.h"

#include <string.h>

constexpr int MAX_STORAGES = 200;

struct storage_t {
    int in_use;
    int building_id;
    building_storage storage;
};

storage_t g_storages[MAX_STORAGES];

void building_storage_clear_all(void) {
    memset(g_storages, 0, MAX_STORAGES * sizeof(storage_t));
}

void building_storage_reset_building_ids(void) {
    for (int i = 1; i < MAX_STORAGES; i++) {
        g_storages[i].building_id = 0;
    }

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_UNUSED)
            continue;

        if (b->type == BUILDING_GRANARY || b->type == BUILDING_STORAGE_YARD) {
            if (b->storage_id) {
                if (g_storages[b->storage_id].building_id) {
                    // storage is already connected to a building: corrupt, create new
                    b->storage_id = building_storage_create(b->type);
                } else {
                    g_storages[b->storage_id].building_id = i;
                }
            }
        }
    }
}

int building_storage_create(int building_type) {
    for (int i = 1; i < MAX_STORAGES; i++) {
        if (!g_storages[i].in_use) {
            memset(&g_storages[i], 0, sizeof(storage_t));
            g_storages[i].in_use = 1;

            // default settings for Pharaoh
            for (int r = 0; r < 36; r++) {
                g_storages[i].storage.resource_state[r] = STORAGE_STATE_PHARAOH_REFUSE;
                g_storages[i].storage.resource_max_accept[r] = 3200;
                g_storages[i].storage.resource_max_get[r] = 3200;
            }
            const resources_list* list = city_resource_get_available();
            for (int r = 0; r < list->size; r++) {
                int resource = list->items[r];
                switch (building_type) {
                case BUILDING_STORAGE_YARD:
                case BUILDING_STORAGE_ROOM:
                    if (resource < 9)
                        g_storages[i].storage.resource_state[resource] = STORAGE_STATE_PHARAOH_REFUSE;
                    else
                        g_storages[i].storage.resource_state[resource] = STORAGE_STATE_PHARAOH_ACCEPT;
                    break;
                case BUILDING_GRANARY:
                    if (resource < 9)
                        g_storages[i].storage.resource_state[resource] = STORAGE_STATE_PHARAOH_ACCEPT;
                    else
                        g_storages[i].storage.resource_state[resource] = STORAGE_STATE_PHARAOH_REFUSE;
                    break;
                }
            }
            return i;
        }
    }
    return 0;
}

int building_storage_restore(int storage_id) {
    if (g_storages[storage_id].in_use) {
        return 0;
    }
    g_storages[storage_id].in_use = 1;
    return storage_id;
}

void building_storage_delete(int storage_id) {
    g_storages[storage_id].in_use = 0;
}

building_storage backup_settings;
static int backup_storage_id = -1;
static bool has_unsaved_changes = false;

void backup_storage_settings(int storage_id) {
    if (backup_storage_id != -1)
        return;
    has_unsaved_changes = false;
    backup_storage_id = storage_id;
    backup_settings = g_storages[storage_id].storage;
}

void restore_storage_settings(bool do_forget_changes) {
    if (do_forget_changes) {
        if (backup_storage_id == -1)
            return;
        g_storages[backup_storage_id].storage = backup_settings;
        storage_settings_backup_reset();
        window_city_show();
    }
}

void storage_settings_backup_check() {
    if (has_unsaved_changes)
        window_popup_dialog_show_confirmation(5, 23, restore_storage_settings);
    else {
        storage_settings_backup_reset();
        window_city_show();
    }
}

void storage_settings_backup_reset() {
    has_unsaved_changes = false;
    backup_storage_id = -1;
}

const building_storage* building_storage_get(int storage_id) {
    return &g_storages[storage_id].storage;
}

void building_storage_toggle_empty_all(int storage_id) {
    has_unsaved_changes = true;
    g_storages[storage_id].storage.empty_all = 1 - g_storages[storage_id].storage.empty_all;
}

void building_storage_cycle_resource_state(int storage_id, int resource_id, bool backwards) {
    has_unsaved_changes = true;
    int state = g_storages[storage_id].storage.resource_state[resource_id];
    if (!backwards) {
        if (state == STORAGE_STATE_PHARAOH_ACCEPT)
            state = STORAGE_STATE_PHARAOH_GET;
        else if (state == STORAGE_STATE_PHARAOH_GET)
            state = STORAGE_STATE_PHARAOH_EMPTY;
        else if (state == STORAGE_STATE_PHARAOH_EMPTY)
            state = STORAGE_STATE_PHARAOH_REFUSE;
        else
            state = STORAGE_STATE_PHARAOH_ACCEPT;
    } else {
        if (state == STORAGE_STATE_PHARAOH_ACCEPT)
            state = STORAGE_STATE_PHARAOH_REFUSE;
        else if (state == STORAGE_STATE_PHARAOH_REFUSE)
            state = STORAGE_STATE_PHARAOH_EMPTY;
        else if (state == STORAGE_STATE_PHARAOH_EMPTY)
            state = STORAGE_STATE_PHARAOH_GET;
        else
            state = STORAGE_STATE_PHARAOH_ACCEPT;
    }

    g_storages[storage_id].storage.resource_state[resource_id] = state;
}

void building_storage_set_permission(int p, building* b) {
    auto& data = g_storages;

    has_unsaved_changes = true;
    const building_storage* s = building_storage_get(b->storage_id);
    int permission_bit = 1 << p;
    int perms = s->permissions;
    perms ^= permission_bit;
    g_storages[b->storage_id].storage.permissions = perms;
}

bool building_storage_get_permission(int p, building* b) {
    const building_storage* s = building_storage_get(b->storage_id);
    int permission_bit = 1 << p;
    return !(s->permissions & permission_bit);
}

void building_storage_increase_decrease_resource_state(int storage_id, int resource_id, bool increase) {
    int state = g_storages[storage_id].storage.resource_state[resource_id];
    if (state == STORAGE_STATE_PHARAOH_ACCEPT) {
        int max_accept = g_storages[storage_id].storage.resource_max_accept[resource_id];
        int old = max_accept;
        if (increase) {
            if (max_accept == 2400)
                max_accept = 3200;
            else if (max_accept == 1600)
                max_accept = 2400;
            else if (max_accept == 800)
                max_accept = 1600;
        } else {
            if (max_accept == 3200)
                max_accept = 2400;
            else if (max_accept == 2400)
                max_accept = 1600;
            else if (max_accept == 1600)
                max_accept = 800;
        }
        g_storages[storage_id].storage.resource_max_accept[resource_id] = max_accept;
        if (old != max_accept)
            has_unsaved_changes = true;

    } else if (state == STORAGE_STATE_PHARAOH_GET) {
        int max_get = g_storages[storage_id].storage.resource_max_get[resource_id];
        int old = max_get;
        if (increase) {
            if (max_get == 2400)
                max_get = 3200;
            else if (max_get == 1600)
                max_get = 2400;
            else if (max_get == 800)
                max_get = 1600;
        } else {
            if (max_get == 3200)
                max_get = 2400;
            else if (max_get == 2400)
                max_get = 1600;
            else if (max_get == 1600)
                max_get = 800;
        }

        g_storages[storage_id].storage.resource_max_get[resource_id] = max_get;
        if (old != max_get)
            has_unsaved_changes = true;
    }
}

void building_storage_accept_none(int storage_id) {
    has_unsaved_changes = true;
    for (int r = RESOURCE_MIN; r < RESOURCES_MAX; r++) {
        g_storages[storage_id].storage.resource_state[r] = STORAGE_STATE_PHARAOH_REFUSE;
    }
}

void building_storage_load_state(buffer* buf) {
}

io_buffer* iob_building_storages = new io_buffer([](io_buffer* iob, size_t version) {
    for (int i = 0; i < MAX_STORAGES; i++) {
        iob->bind(BIND_SIGNATURE_INT32, &g_storages[i].storage.permissions); // Originally unused
        iob->bind(BIND_SIGNATURE_INT32, &g_storages[i].building_id);
        iob->bind(BIND_SIGNATURE_UINT8, &g_storages[i].in_use);
        iob->bind(BIND_SIGNATURE_UINT8, &g_storages[i].storage.empty_all);
        for (int r = 0; r < RESOURCES_MAX; r++) {
            iob->bind(BIND_SIGNATURE_UINT8, &g_storages[i].storage.resource_state[r]);
        }

        iob->bind____skip(6); // unused resource states
        for (int r = 0; r < RESOURCES_MAX; r++) {
            iob->bind(BIND_SIGNATURE_UINT16, &g_storages[i].storage.resource_max_accept[r]);
        }

        for (int r = 0; r < RESOURCES_MAX; r++) {
            iob->bind(BIND_SIGNATURE_UINT16, &g_storages[i].storage.resource_max_get[r]);
        }
        //        iob->bind____skip(144); // ?????
    }
});