/*
 Basic Mission
 Copyright (c) 2021-2026 UGA CSUG LIG

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
				.label = "BALLOON 1 (INISAT F136EAC8)",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00ac92,
				.deveui = 0x0016C001F136EAC8U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
		{
				.label = "BALLOON 2 (INISAT F136E359)",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af5e,
				.deveui = 0x0016c001f136e359U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
		{
				.label = "BALLOON 3 (INISAT FF19582C)",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00aecf,
				.deveui = 0x0016C001FF19582CU,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
		{
				.label = "GROUND 1 (INISAT F136EFFD)",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af2b,
				.deveui = 0x0016c001f136effdU,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
		{
				// Thingsat ProtoSEED (ex BALLOON_1_RANGING)
				.label = "THINGSAT_ProtoSEED_193E63",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00ad15,
				.deveui = 0x0016c001ff193e63U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},

		// TODO add WM1302 for Thingsat ProtoSEED (AMI CMA COMETES WP4 Summer Workshop)

		{
				// iM880a  Label 6F75
				.label = "IMM880A BALLOON 746F75",
				.module = ENDPOINT_SX12xx,
				.devaddr = 0xfc00afeb,
				.deveui = 0x3238353500746f75U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},

		// Entries of gateways sending ranging1 messages

		{
				.label = "ISTATION_0DA1_AIGUILLE_MIDI",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00acfd,
				.deveui = 0x7076ff0056080da1,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 10000,
		},

		{
				.label = "KER110_LAUTARET",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00ae69,
				.deveui = 0x0000024b080602d9,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 10000,
		},

		{
				.label = "LORIX_AIRE_ADOUR",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00ac45,
				.deveui = 0x0016c001ff10d6bb,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 10000,
		},

		{
				.label = "MTCDTIP67_0001DBB_AIGOUAL",
				.module = FIX_GATEWAY,
				.devaddr = 0xfc00ad32,
				.deveui = 0x00800000d0001dbb,
				.nwkskey = POC_NWKSKEY,
				.appskey = POC_APPSKEY,
				.fcntup = 10000,
		},

};
