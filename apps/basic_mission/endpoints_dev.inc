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
				// Thingsat ProtoSEED
				.label = "THINGSAT ProtoSEED 193E63",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00ada4,
				.deveui = 0x0016c001ff193e63U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
		{
				// iM880a  Label 6F75
				.label = "IMM880A_BALLOON_746F75",
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00afeb,
				.deveui = 0x3238353500746f75U,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},

/*
		{
				// Flight model
				.label = "THINGSAT @ STORK1 FM 1",
				.module = THINGSAT_SX1302_BOARD,
				.devaddr = 0xfc00ae69,
				.deveui = 0x0016C001FF1848AEU,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
		{
				// Engineering model
				.label = "THINGSAT @ STORK1 EM 1",
				.module = THINGSAT_SX1302_BOARD,
				.devaddr = 0xfc00ac2c,
				.deveui = 0x0016c001ff1858ccU,
				.nwkskey = DEFAULT_NWKSKEY,
				.appskey = DEFAULT_APPSKEY,
				.fcntup = 0,
		},
*/

		// TODO add 00016c001ff18596 (Thingsat FM aboard Stork - Burn after returning to Earth)

		// TODO add Thingsat EM and FM @ SatRev SOWA
		// TODO add Thingsat EM and FM @ CSUT Nimph


		// Entries of gateways sending ranging1 messages
};
