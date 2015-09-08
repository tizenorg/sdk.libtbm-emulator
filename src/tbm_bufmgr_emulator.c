/*
 * buffer manager for libtbm-emulator
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tbm_bufmgr.h>
#include <tbm_bufmgr_backend.h>
#include <tbm_surface.h>
#include "vigs.h"
#include "tbm_emulator_log.h"
#include <string.h>
#include <stdlib.h>

static uint32_t tbm_bufmgr_emulator_color_format_list[] =
{
    TBM_FORMAT_ARGB8888,
    TBM_FORMAT_XRGB8888,
    TBM_FORMAT_NV21,
    TBM_FORMAT_NV61,
    TBM_FORMAT_YUV420,
};

static tbm_bo_handle get_tbm_bo_handle(struct vigs_drm_gem *gem,
                                       int device)
{
    tbm_bo_handle bo_handle;
    int ret;

    memset(&bo_handle, 0, sizeof(bo_handle));

    switch (device) {
    case TBM_DEVICE_DEFAULT:
    case TBM_DEVICE_2D:
        bo_handle.u32 = gem->handle;
        break;
    case TBM_DEVICE_CPU:
        ret = vigs_drm_gem_map(gem, 1);

        if (ret == 0) {
            bo_handle.ptr = gem->vaddr;
        } else {
            TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_map failed: %s",
                                   strerror(-ret));
        }

        break;
    case TBM_DEVICE_3D:
        TBM_EMULATOR_LOG_ERROR("TBM_DEVICE_3D not supported");
        break;
    case TBM_DEVICE_MM:
        TBM_EMULATOR_LOG_ERROR("TBM_DEVICE_MM not supported");
        break;
    default:
        TBM_EMULATOR_LOG_ERROR("%d not supported", device);
        break;
    }

    return bo_handle;
}

static void tbm_bufmgr_emulator_deinit(void *priv)
{
    struct vigs_drm_device *drm_dev = priv;

    TBM_EMULATOR_LOG_DEBUG("enter");

    vigs_drm_device_destroy(drm_dev);
}

static int tbm_bufmgr_emulator_bo_size(tbm_bo bo)
{
    struct vigs_drm_surface *sfc;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    return sfc->gem.size;
}

static void *tbm_bufmgr_emulator_bo_alloc(tbm_bo bo, int size, int flags)
{
    struct vigs_drm_device *drm_dev;
    struct vigs_drm_surface *sfc;
    uint32_t width = 2048, height;
    int ret;

    TBM_EMULATOR_LOG_DEBUG("size = %d, flags = 0x%X", size, flags);

    drm_dev = (struct vigs_drm_device*)tbm_backend_get_bufmgr_priv(bo);

    height = ((uint32_t)size + (width * 4) - 1) / (width * 4);

    ret = vigs_drm_surface_create(drm_dev,
                                  width, height,
                                  width * 4,
                                  vigs_drm_surface_bgra8888, 0,
                                  &sfc);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_suface_create failed: %s",
                               strerror(-ret));
        return NULL;
    }

    return sfc;
}

static void tbm_bufmgr_emulator_bo_free(tbm_bo bo)
{
    struct vigs_drm_surface *sfc;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    vigs_drm_gem_unref(&sfc->gem);
}

static void *tbm_bufmgr_emulator_bo_import(tbm_bo bo, unsigned int key)
{
    struct vigs_drm_device *drm_dev;
    int ret;
    struct vigs_drm_surface *sfc;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, key = %u", bo, key);

    drm_dev = (struct vigs_drm_device*)tbm_backend_get_bufmgr_priv(bo);

    ret = vigs_drm_surface_open(drm_dev, key, &sfc);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_surface_open failed for key %u: %s",
                               key,
                               strerror(-ret));
        return NULL;
    }

    TBM_EMULATOR_LOG_DEBUG("handle = %u", sfc->gem.handle);

    return sfc;
}

static unsigned int tbm_bufmgr_emulator_bo_export(tbm_bo bo)
{
    struct vigs_drm_surface *sfc;
    int ret;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    ret = vigs_drm_gem_get_name(&sfc->gem);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_get_name failed: %s",
                               strerror(-ret));
        return 0;
    }

    return sfc->gem.name;
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_get_handle(tbm_bo bo, int device)
{
    struct vigs_drm_surface *sfc;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, device = %d", bo, device);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    return get_tbm_bo_handle(&sfc->gem, device);
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_map(tbm_bo bo, int device, int opt)
{
    struct vigs_drm_surface *sfc;
    tbm_bo_handle handle;
    uint32_t saf = 0;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, device = %d, opt = %d", bo, device, opt);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    handle = get_tbm_bo_handle(&sfc->gem, device);

    if (!handle.ptr) {
        return handle;
    }

    if ((opt & TBM_OPTION_READ) != 0) {
        saf |= VIGS_DRM_SAF_READ;
    }

    if ((opt & TBM_OPTION_WRITE) != 0) {
        saf |= VIGS_DRM_SAF_WRITE;
    }

    vigs_drm_surface_start_access(sfc, saf);

    return handle;
}

static int tbm_bufmgr_emulator_bo_unmap(tbm_bo bo)
{
    struct vigs_drm_surface *sfc;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    vigs_drm_surface_end_access(sfc, 1);

    return 1;
}

static int tbm_bufmgr_emulator_bo_cache_flush(tbm_bo bo, int flags)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p, flags = %d", bo, flags);
    return 1;
}

static int tbm_bufmgr_emulator_bo_lock(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);
    return 1;
}

static int tbm_bufmgr_emulator_bo_unlock(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);
    return 1;
}

static int tbm_bufmgr_emulator_bo_get_global_key(tbm_bo bo)
{
    struct vigs_drm_surface *sfc;
    int ret;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    sfc = (struct vigs_drm_surface*)tbm_backend_get_bo_priv(bo);

    ret = vigs_drm_gem_get_name(&sfc->gem);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_get_name failed: %s",
                               strerror(-ret));
        return 0;
    }

    return sfc->gem.name;
}

static int tbm_bufmgr_emulator_surface_get_plane_data(tbm_surface_h surface, int width, int height, tbm_format format, int plane_idx, uint32_t *size, uint32_t *offset, uint32_t *pitch)
{
    *size = 0;
    *offset = 0;
    *pitch = 0;

    switch(format) {
    case TBM_FORMAT_XRGB8888:
    case TBM_FORMAT_ARGB8888:
        *size = width * height * 4;
        *offset = 0;
        *pitch = width * 4;
        return 1;
    case TBM_FORMAT_NV21:
        if (plane_idx == 0) {
            *size = width * height;
            *offset = 0;
            *pitch = width;
        } else if (plane_idx == 1) {
            *size = width * (height >> 1);
            *offset = width * height;
            *pitch = width;
        } else {
            return 0;
        }
        return 1;
    case TBM_FORMAT_NV61:
        if (plane_idx == 0) {
            *size = width * height;
            *offset = 0;
            *pitch = width;
        } else if (plane_idx == 1) {
            *size = width * height;
            *offset = width * height;
            *pitch = width;
        } else {
            return 0;
        }
        return 1;
    case TBM_FORMAT_YUV420:
        if (plane_idx == 0) {
            *size = width * height;
            *offset = 0;
            *pitch = width;
        } else if (plane_idx == 1) {
            *size = (width * height) >> 2;
            *offset = width * height;
            *pitch = width >> 1 ;
        } else if (plane_idx == 2) {
            *size = (width * height) >> 2;
            *offset = (width * height) + (width  * height >> 2);
            *pitch = width >> 1;
        } else {
            return 0;
        }
        return 1;
    default:
        return 0;
    }
}

static int tbm_bufmgr_emulator_surface_get_size(tbm_surface_h surface, int width, int height, tbm_format format)
{
    int bpp;

    switch(format) {
    case TBM_FORMAT_XRGB8888:
    case TBM_FORMAT_ARGB8888:
        bpp = 32;
        break;
    /* NV21 : Y/CrCb 4:2:0 */
    /* YUV420 : YUV 4:2:0 */
    case TBM_FORMAT_NV21:
    case TBM_FORMAT_YUV420:
        bpp = 12;
        break;
    /* NV61 : Y/CrCb 4:2:2 */
    case TBM_FORMAT_NV61:
        bpp = 16;
        break;
    default:
        return 0;
    }
    return (width * height * bpp) >> 3;
}

static int tbm_bufmgr_emulator_surface_supported_format(uint32_t **formats, uint32_t *num)
{
    uint32_t *color_formats;

    color_formats = (uint32_t*)calloc(1, sizeof(tbm_bufmgr_emulator_color_format_list));

    if (!color_formats) {
        return 0;
    }

    memcpy(color_formats,
           tbm_bufmgr_emulator_color_format_list,
           sizeof(tbm_bufmgr_emulator_color_format_list));

    *formats = color_formats;
    *num = sizeof(tbm_bufmgr_emulator_color_format_list)/sizeof(tbm_bufmgr_emulator_color_format_list[0]);

    return 1;
}


MODULEINITPPROTO(tbm_bufmgr_emulator_init);

static TBMModuleVersionInfo EmulatorVersRec =
{
    "emulator",
    "Samsung",
    TBM_ABI_VERSION,
};

TBMModuleData tbmModuleData = { &EmulatorVersRec, tbm_bufmgr_emulator_init };

int tbm_bufmgr_emulator_init(tbm_bufmgr bufmgr, int fd)
{
    int ret = 0;
    struct vigs_drm_device *drm_dev = NULL;
    tbm_bufmgr_backend backend = NULL;

    TBM_EMULATOR_LOG_DEBUG("enter");

    if (!bufmgr) {
        return 0;
    }

    ret = vigs_drm_device_create(fd, &drm_dev);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_device_create failed: %s", strerror(-ret));
        goto fail;
    }

    backend = tbm_backend_alloc();

    if (!backend) {
        TBM_EMULATOR_LOG_ERROR("tbm_backend_alloc failed");
        goto fail;
    }

    backend->flags = TBM_CACHE_CTRL_BACKEND|TBM_LOCK_CTRL_BACKEND;
    backend->priv = (void*)drm_dev;
    backend->bufmgr_deinit = tbm_bufmgr_emulator_deinit;
    backend->bo_size = tbm_bufmgr_emulator_bo_size;
    backend->bo_alloc = tbm_bufmgr_emulator_bo_alloc;
    backend->bo_free = tbm_bufmgr_emulator_bo_free;
    backend->bo_import = tbm_bufmgr_emulator_bo_import;
    backend->bo_export = tbm_bufmgr_emulator_bo_export;
    backend->bo_get_handle = tbm_bufmgr_emulator_bo_get_handle;
    backend->bo_map = tbm_bufmgr_emulator_bo_map;
    backend->bo_unmap = tbm_bufmgr_emulator_bo_unmap;
    backend->bo_cache_flush = tbm_bufmgr_emulator_bo_cache_flush;
    backend->bo_get_global_key = tbm_bufmgr_emulator_bo_get_global_key;
    backend->bo_lock = tbm_bufmgr_emulator_bo_lock;
    backend->bo_unlock = tbm_bufmgr_emulator_bo_unlock;
    backend->surface_get_plane_data = tbm_bufmgr_emulator_surface_get_plane_data;
    backend->surface_get_size = tbm_bufmgr_emulator_surface_get_size;
    backend->surface_supported_format = tbm_bufmgr_emulator_surface_supported_format;

    if (!tbm_backend_init(bufmgr, backend)) {
        TBM_EMULATOR_LOG_ERROR("tbm_backend_init failed");
        goto fail;
    }

    TBM_EMULATOR_LOG_INFO("initialized");

    return 1;

fail:
    if (backend) {
        tbm_backend_free(backend);
    }

    if (drm_dev) {
        vigs_drm_device_destroy(drm_dev);
    }

    return 0;
}
