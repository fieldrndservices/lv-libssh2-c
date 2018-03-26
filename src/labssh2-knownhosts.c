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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "libssh2.h"

#include "labssh2.h"
#include "labssh2-private.h"
#include "labssh2-session-private.h"
#include "labssh2-status-private.h"
#include "labssh2-knownhost-private.h"
#include "labssh2-knownhosts-private.h"

static const char* NULL_KNOWNHOSTS = "The known hosts cannot be NULL";
static const char* NULL_KNOWNHOST = "The known host cannot be NULL";
static const char* NULL_FILE = "The file cannot be NULL";
static const char* NULL_LINE = "The line cannot be NULL";

labssh2_knownhosts_t*
labssh2_knownhosts_create(
    labssh2_t* ctx,
    labssh2_session_t* session
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return NULL;
    }
    if (session == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_create";
        ctx->message = "The session cannot be NULL";
        return NULL;
    }
    LIBSSH2_KNOWNHOSTS* inner = libssh2_knownhost_init(session->inner);
    if (inner == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_init";
        ctx->message = "Errors occurred during the creation of a new known hosts";
        return NULL;
    }
    labssh2_knownhosts_t* knownhosts = malloc(sizeof(labssh2_knownhosts_t));
    if (knownhosts == NULL) {
        libssh2_knownhost_free(inner);
        ctx->status = LABSSH2_STATUS_ERROR_MEMORY;
        ctx->source = "malloc";
        ctx->message = "Could not allocate memory to create a known hosts";
        return NULL;
    }
    knownhosts->inner = inner;
    knownhosts->prev = NULL;
    return knownhosts;
}

void
labssh2_knownhosts_destroy(
    labssh2_knownhosts_t* knownhosts
) {
    assert(knownhosts);
    libssh2_knownhost_free(knownhosts->inner);
    knownhosts->inner = NULL;
    knownhosts->prev = NULL;
    free(knownhosts);
}

bool
labssh2_knownhosts_get(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    labssh2_knownhost_t* knownhost
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return true;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_get";
        ctx->message = NULL_KNOWNHOSTS;
        return true; 
    }
    if (knownhost == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_get";
        ctx->message = NULL_KNOWNHOST;
        return true; 
    }
    int result = libssh2_knownhost_get(
        knownhosts->inner, 
        &knownhost->inner, 
        knownhosts->prev->inner
    );
    switch (result) {
        case 0: return false;
        case 1: return true;
        default:
            ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
            ctx->source = "libssh2_knownhost_get";
            ctx->message = labssh2_status_error_to_message(result);
            return true;
    }
}

void
labssh2_knownhosts_add(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    const char* name,
    const char* salt,
    const char* key,
    const size_t key_len,
    const char* comment,
    const size_t comment_len,
    const int type_mask,
    labssh2_knownhost_t* knownhost
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_add";
        ctx->message = NULL_KNOWNHOSTS;
        return; 
    }
    if (knownhost == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_add";
        ctx->message = NULL_KNOWNHOST;
        return; 
    }
    int result = libssh2_knownhost_addc(
        knownhosts->inner, 
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
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_add";
        ctx->message = labssh2_status_error_to_message(result);
    }
}

int
labssh2_knownhosts_type_mask(
    labssh2_knownhost_types_t type,
    labssh2_knownhost_key_encodings_t encoding,
    labssh2_knownhost_key_algorithms_t algorithm
) {
    return type | encoding | algorithm;
}

labssh2_knownhosts_check_result_t
labssh2_knownhosts_check(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    const char* host,
    const int port,
    const char* key,
    const size_t key_len,
    const int type_mask,
    labssh2_knownhost_t* knownhost
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return LABSSH2_KNOWNHOSTS_CHECK_RESULT_FAILURE;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_check";
        ctx->message = NULL_KNOWNHOSTS;
        return LABSSH2_KNOWNHOSTS_CHECK_RESULT_FAILURE;
    }
    if (knownhost == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_check";
        ctx->message = NULL_KNOWNHOST;
        return LABSSH2_KNOWNHOSTS_CHECK_RESULT_FAILURE;
    }
    int result = libssh2_knownhost_checkp(
        knownhosts->inner, 
        host, 
        port, 
        key, 
        key_len, 
        type_mask, 
        &knownhost->inner
    );
    switch (result) {
        case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND: return LABSSH2_KNOWNHOSTS_CHECK_RESULT_NOT_FOUND;
        case LIBSSH2_KNOWNHOST_CHECK_MATCH: return LABSSH2_KNOWNHOSTS_CHECK_RESULT_MATCH;
        case LIBSSH2_KNOWNHOST_CHECK_MISMATCH: return LABSSH2_KNOWNHOSTS_CHECK_RESULT_MISMATCH;
        default:
            ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
            ctx->source = "libssh2_knownhost_checkp";
            ctx->message = labssh2_status_error_to_message(LIBSSH2_KNOWNHOST_CHECK_FAILURE);
            return LABSSH2_KNOWNHOSTS_CHECK_RESULT_FAILURE;
    }
}

void
labssh2_knownhosts_delete(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    labssh2_knownhost_t* knownhost
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_delete";
        ctx->message = NULL_KNOWNHOSTS;
        return;
    }
    if (knownhost == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_delete";
        ctx->message = NULL_KNOWNHOST;
        return;
    }
    int result = libssh2_knownhost_del(
        knownhosts->inner, 
        knownhost->inner
    );
    if (result != 0) {
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_del";
        ctx->message = labssh2_status_error_to_message(result);
    }
}

int
labssh2_knownhosts_read_file(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    const char* file
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return 0;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_read_file";
        ctx->message = NULL_KNOWNHOSTS;
        return 0;
    }
    if (file == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_read_file";
        ctx->message = NULL_FILE;
        return 0;
    }
    int result = libssh2_knownhost_readfile(
        knownhosts->inner, 
        file,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result < 0) {
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_readfile";
        ctx->message = labssh2_status_error_to_message(result);
    }
    return result;
}

void
labssh2_knownhosts_read_line(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    const char* line,
    const size_t line_len
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_read_line";
        ctx->message = NULL_KNOWNHOSTS;
        return;
    }
    if (line == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_read_line";
        ctx->message = NULL_LINE;
        return;
    }
    int result = libssh2_knownhost_readline(
        knownhosts->inner, 
        line,
        line_len,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result != 0) {
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_readfile";
        ctx->message = labssh2_status_error_to_message(result);
    }
}

void
labssh2_knownhosts_write_file(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    const char* file
) {
    assert(ctx);
    if (labssh2_is_err(ctx)) {
        return;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_write_file";
        ctx->message = NULL_KNOWNHOSTS;
        return;
    }
    if (file == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_write_file";
        ctx->message = NULL_FILE;
        return;
    }
    int result = libssh2_knownhost_writefile(
        knownhosts->inner, 
        file,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result != 0) {
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_writefile";
        ctx->message = labssh2_status_error_to_message(result);
    }
}

size_t
labssh2_knownhosts_write_line(
    labssh2_t* ctx,
    labssh2_knownhosts_t* knownhosts,
    labssh2_knownhost_t* knownhost,
    char* buffer,
    size_t buffer_len
) {
    assert(ctx);
    size_t out_len = 0;
    if (labssh2_is_err(ctx)) {
        return out_len;
    }
    if (knownhosts == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_write_line";
        ctx->message = NULL_KNOWNHOSTS;
        return out_len;
    }
    if (knownhost == NULL) {
        ctx->status = LABSSH2_STATUS_ERROR_NULL_VALUE;
        ctx->source = "labssh2_knownhosts_write_line";
        ctx->message = NULL_KNOWNHOST;
        return out_len;
    }
    int result = libssh2_knownhost_writeline(
        knownhosts->inner, 
        knownhost->inner,
        buffer,
        buffer_len,
        &out_len,
        LIBSSH2_KNOWNHOST_FILE_OPENSSH
    );
    if (result < 0) {
        ctx->status = LABSSH2_STATUS_ERROR_KNOWNHOSTS;
        ctx->source = "libssh2_knownhost_writefile";
        ctx->message = labssh2_status_error_to_message(result);
        return out_len;
    }
    return out_len;
}

