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


#ifdef MODULE_LWIP_MQTT

#include "lwip/apps/mqtt.h"

// https://github.com/lwip-tcpip/lwip/blob/master/test/unit/mqtt/test_mqtt.c

#else
typedef int dont_be_pedantic;
#endif

/** @} */
