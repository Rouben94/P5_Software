/**
 * blueseidon_test_example_common.h 
 * Copyright (C) 2019 blueseidon

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BLUESEIDON_TEST_EXAMPLE_COMMON_H__
#define BLUESEIDON_TEST_EXAMPLE_COMMON_H__

/**
 * @defgroup BLUESEIDON_TEST_EXAMPLE_COMMON Common definitions for the blueseidon test example
 * @{
 */

/** Number of active servers.
 * Note: If the value of SERVER_NODE_COUNT is increased, you may need to scale up the the replay
 * protection list size (@ref REPLAY_CACHE_ENTRIES), by the appropriate amount, for the provisioner and
 * client examples. For the provisioner example to work as expected, its replay protection list size should
 * be greater than or equal to the total number of nodes it is going to configure after provisioning them.
 * The replay protection list size of the client example should be greater than or equal to the total
 * number of unicast addresses in the network that it can receive a message from.
 */
#define SERVER_NODE_COUNT (30)
#if SERVER_NODE_COUNT > 30
#error Maximum 30 servers currently supported by client example.
#endif

/** Number of active clients nodes. */
#define CLIENT_NODE_COUNT            (1)

/** Number of On-Off client models on the Switch Node */
#define CLIENT_MODEL_INSTANCE_COUNT  (2)

/** Number of group address being used in this example */
#define GROUP_ADDR_COUNT             (2)

/** Static authentication data */
#define STATIC_AUTH_DATA {0x6E, 0x6F, 0x72, 0x64, 0x69, 0x63, 0x5F, 0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x5F, 0x31}


/** @} end of Common definitions for the blueseidon test example */

#endif /* BLUESEIDON_TEST_EXAMPLE_COMMON_H__ */
