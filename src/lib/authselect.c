/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2017 Red Hat

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stddef.h>
#include <stdlib.h>

#include "authselect.h"

#include "lib/authselect_private.h"
#include "lib/authselect_paths.h"
#include "lib/util/string_array.h"
#include "lib/files/files.h"

_PUBLIC_ void
authselect_set_debug_fn(authselect_debug_fn fn, void *pvt)
{
    set_debug_fn(fn, pvt);
}

_PUBLIC_ int
authselect_check_conf(bool *_is_valid)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_config_read(&profile_id, &features);
    if (ret == ENOENT) {
        *_is_valid = authselect_config_validate_non_existing();
        return ENOENT;
    } if (ret != EOK) {
        return ret;
    }

    *_is_valid = authselect_config_validate_existing(profile_id,
                                                     (const char **)features);

    free(profile_id);
    string_array_free(features);

    return EOK;
}

_PUBLIC_ int
authselect_current(char **_profile_id,
                   char ***_features)
{
    return authselect_config_read(_profile_id, _features);
}

_PUBLIC_ void
authselect_features_free(char **features)
{
    string_array_free(features);
}

_PUBLIC_ char **
authselect_list()
{
    struct authselect_dir *profile = NULL;
    struct authselect_dir *vendor = NULL;
    struct authselect_dir *custom = NULL;
    char **profiles = NULL;
    errno_t ret;

    ret = authselect_dir_read(DIR_DEFAULT_PROFILES, &profile);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_dir_read(DIR_VENDOR_PROFILES, &vendor);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_dir_read(DIR_CUSTOM_PROFILES, &custom);
    if (ret != EOK) {
        goto done;
    }

    profiles = authselect_merge_profiles(profile, vendor, custom);

done:
    authselect_dir_free(profile);
    authselect_dir_free(vendor);
    authselect_dir_free(custom);

    return profiles;
}

_PUBLIC_ void
authselect_list_free(char **profile_ids)
{
    string_array_free(profile_ids);
}

_PUBLIC_ int
authselect_cat(const char *profile_id,
               const char **features,
               struct authselect_files **_files)
{
    struct authselect_profile *profile;
    struct authselect_files *files;
    errno_t ret;

    ret = authselect_profile(profile_id, &profile);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_system_generate(features, &profile->files, &files);
    authselect_profile_free(profile);
    if (ret != EOK) {
        return ret;
    }

    *_files = files;

    return EOK;
}
