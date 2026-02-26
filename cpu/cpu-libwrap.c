#include <stdlib.h>

#include "cpu-libwrap.h"
#include "log.h"

static const char* LIBCUDA_PATH = "/usr/local/cuda/lib64/libcudart.so";
static void *so_handle;

inline void* libwrap_get_sohandle()
{
    if (!so_handle) {
        if ( !(so_handle = dlopen(LIBCUDA_PATH, RTLD_LAZY)) ) {
            LOGE(LOG_ERROR, "%s", dlerror());
            so_handle = NULL;
            return 0;
        }
    }
    return so_handle;
}

static const char* LIBNVML_PATH = "libnvidia-ml.so.1";
static void *nvml_so_handle;

inline void* libwrap_get_nvml_handle()
{
    if (!nvml_so_handle) {
        if ( !(nvml_so_handle = dlopen(LIBNVML_PATH, RTLD_LAZY | RTLD_GLOBAL)) ) {
            LOGE(LOG_ERROR, "[nvml] %s", dlerror());
            nvml_so_handle = NULL;
            return 0;
        }
    }
    return nvml_so_handle;
}

inline void libwrap_pre_call(char *ret, char *name, char *parameters)
{
    LOG(LOG_DEBUG, "%s", name);
}
inline void libwrap_post_call(char *ret, char *name, char *parameters)
{
    LOG(LOG_DEBUG, "%s", name);
}
