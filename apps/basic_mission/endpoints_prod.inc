/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

// WARNING : This must be changed for production !!!!!!
#define DEFAULT_NWKSKEY 	{0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78,0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78}
#define DEFAULT_APPSKEY 	{0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78,0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78}


// 44024241ed4ce9a68c6a8bc055233fd3
#define POC_NWKSKEY			{0x44, 0x02, 0x42, 0x41, 0xed, 0x4c, 0xe9, 0xa6, 0x8c, 0x6a, 0x8b, 0xc0, 0x55, 0x23, 0x3f, 0xd3}
// ec925802ae430ca77fd3dd73cb2cc588
#define POC_APPSKEY			{0xec, 0x92, 0x58, 0x02, 0xae, 0x43, 0x0c, 0xa7, 0x7f, 0xd3, 0xdd, 0x73, 0xcb, 0x2c, 0xc5, 0x88}


/*
 * Me and list of my friends
 */
static lorawan_endpoint_t lorawan_endpoints[] = {
		{
				// ORANGE
				.label = "BALLOON 1",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0x1f22a9a9,
				.deveui = 0x0016C001F136EAC8,
				// 3c9fe3664860c4b3561c2b73a2aaa7c8
				.nwkskey = {0x3c, 0x9f, 0xe3, 0x66, 0x48, 0x60, 0xc4, 0xb3, 0x56, 0x1c, 0x2b, 0x73, 0xa2, 0xaa, 0xa7, 0xc8},
				// 3935ab7b08c5213e7e19576e9821efe3
				.appskey = {0x39, 0x35, 0xab, 0x7b, 0x08, 0xc5, 0x21, 0x3e, 0x7e, 0x19, 0x57, 0x6e, 0x98, 0x21, 0xef, 0xe3},
/*
 * gmake BOARD=im880b OTAA=1 DEVEUI=0016C001F136EAC8 APPEUI=0000000000000000 APPKEY=CE31CBF37F3F4502B587EE7D72856F3B
 * [otaa] DevAddr: 1f22a9a9
 * [otaa] NwkSKey:3c9fe3664860c4b3561c2b73a2aaa7c8
 * [otaa] AppSKey:3935ab7b08c5213e7e19576e9821efe3
 */

				.fcntup = 100,
		},
		{
				// NETMORE
				.label = "BALLOON 2",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0x1241155c,
				.deveui = 0x0016c001f136e359U,
				.nwkskey = {0xca, 0xe3, 0xc1, 0xbd, 0xd3, 0x48, 0x8f, 0xe3, 0x5f, 0x5a, 0x6f, 0xd5, 0x77, 0x67, 0x77, 0x39},
				.appskey = {0x6b, 0x61, 0xdf, 0x52, 0x7e, 0x56, 0xa0, 0x88, 0xb0, 0x00, 0xe3, 0x6c, 0x10, 0x5b, 0xde, 0x54},
				.fcntup = 100,
/*
 * gmake BOARD=im880b OTAA=1 DEVEUI=0016c001f136e359 APPEUI=0000000000000000 APPKEY=CE31CBF37F3F4502B587EE7D72856F3B
[otaa] DevAddr: 1241155c
[otaa] NwkSKey:cae3c1bdd3488fe35f5a6fd577677739
[otaa] AppSKey:6b61df527e56a088b000e36c105bde54
 */
		},
		{
				// TTN ABP
				.label = "BALLOON 3",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0x260B89AD,
				.deveui = 0x0016C001FF19582C,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 100,
		},
		{
				.label = "OTHER",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af2b,
				.deveui = 0x0016c001f136effd,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 300,
		},

		// Entries of gateways sending ranging1 messages

		{
				.label = "ISTATION_0DA1_AIGUILLE_MIDI",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00acfd,
				.deveui = 0x7076ff0056080da1,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 1,
		},

		{
				.label = "KER110_LAUTARET",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00ae69,
				.deveui = 0x0000024b080602d9,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 1,
		},

		{
				.label = "MTCDTIP2_021E9B_MONT_AIGOUAL",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00ad53,
				.deveui = 0x0080000000021e9b,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 1,
		},

		{
				.label = "MTCDT67_AIRE_ADOUR",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00af2f,
				.deveui = 0x00800000a00064e9,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 1,
		},


		{
				.label = "MTCDT67_MONT_AIGOUAL",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00ad42,
				.deveui = 0x00800000a000e605,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 1,
		},

};
