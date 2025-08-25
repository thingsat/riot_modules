/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

//
// WARNING : This must be changed for production !!!!!! See endpoints_prod.inc
//

// BABECAFE12345678BABECAFE12345678
#define DEFAULT_NWKSKEY 	{0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78,0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78}
// CAFEBABE12345678CAFEBABE12345678
#define DEFAULT_APPSKEY 	{0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78,0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78}

/*
 * Me and list of my friends
 */
static lorawan_endpoint_t lorawan_endpoints[] = {
		{
				.label = "BALLOON 1",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00ac92,
				.deveui = 0x0016C001F136EAC8,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 100,
		},
		{
				.label = "BALLOON 2",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af5e,
				.deveui = 0x0016c001f136e359U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 100,
		},
		{
				.label = "BALLOON 3",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00aecf,
				.deveui = 0x0016C001FF19582C,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 100,
		},
		{
				.label = "OTHER",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af2b,
				.deveui = 0x0016c001f136effd,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 100,
		},


		// Entries of gateways sending ranging1 messages
};
