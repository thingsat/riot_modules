/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 * @}
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "od.h"
#include "net/af.h"
#include "net/sock/async/event.h"
#include "net/sock/udp.h"
#include "net/sock/util.h"
#include "shell.h"
#include "test_utils/expect.h"
#include "thread.h"
#include "ztimer.h"


#ifdef MODULE_LWIP_HTTPCLIENT

#include "lwip/apps/http_client.h"

//err_t httpc_get_file(const ip_addr_t* server_addr, u16_t port, const char* uri, const httpc_connection_t *settings,
//                     altcp_recv_fn recv_fn, void* callback_arg, httpc_state_t **connection);


#else
typedef int dont_be_pedantic;
#endif

/** @} */
