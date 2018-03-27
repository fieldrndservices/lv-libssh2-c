/*
 * LabSSH2 - A LabVIEW-Friendly C library for libssh2 
 *
 * Copyright (c) 2018 Field R&D Services, LLC. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * withoutmodification, are permitted provided that the following conditions
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 3. Neither the name of the Field R&D Services nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY Field R&D Services, LLC ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Field R&D Services, LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributor(s):
 *   Christopher R. Field <chris@fieldrndservices.com>
 */

#include <stdbool.h>
#include <stdlib.h>

#include "libssh2.h"

#include "labssh2.h"
#include "labssh2-session-private.h"
#include "labssh2-knownhost-private.h"
#include "labssh2-knownhosts-private.h"

labssh2_status_t
labssh2_knownhosts_create(
    labssh2_session_t* session,
    labssh2_knownhosts_t* handle
) {
    if (session == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    LIBSSH2_KNOWNHOSTS* inner = libssh2_knownhost_init(session->inner);
    if (inner == NULL) {
        return LABSSH2_STATUS_ERROR_GENERIC;
    }
    labssh2_knownhosts_t* knownhosts = malloc(sizeof(labssh2_knownhosts_t));
    if (knownhosts == NULL) {
        libssh2_knownhost_free(inner);
        return LABSSH2_STATUS_ERROR_MALLOC;
    }
    knownhosts->inner = inner;
    knownhosts->prev = NULL;
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_destroy(
    labssh2_knownhosts_t* handle
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    libssh2_knownhost_free(handle->inner);
    handle->inner = NULL;
    handle->prev = NULL;
    free(handle);
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_get(
    labssh2_knownhosts_t* handle,
    labssh2_knownhost_t* knownhost
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE; 
    }
    if (knownhost == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE; 
    }
    int result = libssh2_knownhost_get(
        handle->inner, 
        &knownhost->inner, 
        handle->prev->inner
    );
    switch (result) {
        case 0: return LABSSH2_STATUS_OK;
        case 1: return LABSSH2_STATUS_END_OF_HOSTS;
        default: return LABSSH2_STATUS_ERROR_GENERIC;
    }
}

labssh2_status_t
labssh2_knownhosts_add(
    labssh2_knownhosts_t* handle,
    const char* name,
    const char* salt,
    const char* key,
    const size_t key_len,
    const char* comment,
    const size_t comment_len,
    const int type_mask,
    labssh2_knownhost_t* knownhost
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (knownhost == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE; 
    }
    int result = libssh2_knownhost_addc(
        handle->inner, 
        name, 
        salt, 
        key, 
        key_len, 
        comment, 
        comment_len, 
        type_mask, 
        &knownhost->inner
    );
    if (result != 0) {
        return LABSSH2_STATUS_ERROR_GENERIC;
    }
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_type_mask(
    labssh2_knownhost_name_types_t type,
    labssh2_knownhost_key_encodings_t encoding,
    labssh2_knownhost_key_algorithms_t algorithm,
    int* type_mask
) {
    *type_mask = 0;
    switch (type) {
        case LABSSH2_KNOWNHOST_NAME_TYPE_PLAIN: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_TYPE_PLAIN; break;
        case LABSSH2_KNOWNHOST_NAME_TYPE_SHA1: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_TYPE_SHA1; break;
        case LABSSH2_KNOWNHOST_NAME_TYPE_CUSTOM: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_TYPE_CUSTOM; break;
        default: return LABSSH2_STATUS_ERROR_UNKNOWN_NAME_TYPE;
    }
    switch (encoding) {
        case LABSSH2_KNOWNHOST_KEY_ENCODING_RAW: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_KEYENC_RAW; break;
        case LABSSH2_KNOWNHOST_KEY_ENCODING_BASE64: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_KEYENC_BASE64; break;
        default: return LABSSH2_STATUS_ERROR_UNKNOWN_KEY_ENCODING;
    }
    switch (algorithm) {
        case LABSSH2_KNOWNHOST_KEY_ALGORITHM_RSA1: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_KEY_RSA1; break;
        case LABSSH2_KNOWNHOST_KEY_ALGORITHM_SSHRSA: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_KEY_SSHRSA; break;
        case LABSSH2_KNOWNHOST_KEY_ALGORITHM_SSHDSS: *type_mask = *type_mask | LIBSSH2_KNOWNHOST_KEY_SSHDSS; break;
        default: return LABSSH2_STATUS_ERROR_UNKNOWN_KEY_ALGORITHM;
    }
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_check(
    labssh2_knownhosts_t* handle,
    const char* host,
    const int port,
    const char* key,
    const size_t key_len,
    const int type_mask,
    labssh2_knownhost_t* knownhost
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (knownhost == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    int result = libssh2_knownhost_checkp(
        handle->inner, 
        host, 
        port, 
        key, 
        key_len, 
        type_mask, 
        &knownhost->inner
    );
    switch (result) {
        case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND: return LABSSH2_STATUS_NOT_FOUND;
        case LIBSSH2_KNOWNHOST_CHECK_MATCH: return LABSSH2_STATUS_MATCH;
        case LIBSSH2_KNOWNHOST_CHECK_MISMATCH: return LABSSH2_STATUS_MISMATCH;
        default: return LABSSH2_STATUS_ERROR_GENERIC;
    }
}

labssh2_status_t
labssh2_knownhosts_delete(
    labssh2_knownhosts_t* handle,
    labssh2_knownhost_t* knownhost
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (knownhost == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    int result = libssh2_knownhost_del(
        handle->inner, 
        knownhost->inner
    );
    if (result != 0) {
        return LABSSH2_STATUS_ERROR_GENERIC;
    }
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_read_file(
    labssh2_knownhosts_t* handle,
    const char* file,
    size_t* count
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (file == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    int result = libssh2_knownhost_readfile(
        handle->inner, 
        file,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result < 0) {
        return LABSSH2_STATUS_ERROR_GENERIC; // TODO: Change to function that converts libssh2 result to error
    }
    *count = result;
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_read_line(
    labssh2_knownhosts_t* handle,
    const char* line,
    const size_t line_len
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (line == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    int result = libssh2_knownhost_readline(
        handle->inner, 
        line,
        line_len,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result != 0) {
        return LABSSH2_STATUS_ERROR_GENERIC;
    }
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_write_file(
    labssh2_knownhosts_t* handle,
    const char* file
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (file == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    int result = libssh2_knownhost_writefile(
        handle->inner, 
        file,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result != 0) {
        return LABSSH2_STATUS_ERROR_GENERIC;
    }
    return LABSSH2_STATUS_OK;
}

labssh2_status_t
labssh2_knownhosts_write_line(
    labssh2_knownhosts_t* handle,
    labssh2_knownhost_t* knownhost,
    char* line,
    const size_t line_len,
    size_t* len
) {
    if (handle == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    if (knownhost == NULL) {
        return LABSSH2_STATUS_ERROR_NULL_VALUE;
    }
    int result = libssh2_knownhost_writeline(
        handle->inner, 
        knownhost->inner,
        line,
        line_len,
        len,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result < 0) {
        return LABSSH2_STATUS_ERROR_GENERIC; // TODO: Convert from libssh2 error instead of generic
    }
    return LABSSH2_STATUS_OK;
}

