
#include <math.h>

// Value used for the conversion of the position from DMS to decimal.
static const int32_t MaxNorthPosition = 8388607;  // 2^23 - 1
static const int32_t MaxSouthPosition = 8388608;  // -2^23
static const int32_t MaxEastPosition  = 8388607;  // 2^23 - 1
static const int32_t MaxWestPosition  = 8388608;  // -2^23


// Convert GPS positions from double to binary values.
uint32_t gps_pack_latitude_f_to_i24(const float lat)
{
    int32_t latitude_bin;

    if (lat >= 0) { // North
        latitude_bin = (lat * MaxNorthPosition) / 90;
    } else {        // South
        latitude_bin = (lat * MaxSouthPosition) / 90;
    }
    return (uint32_t) (latitude_bin >> 8);
}

// Convert GPS positions from double to binary values.
uint32_t gps_pack_longitude_f_to_i24(const float lon)
{
    int32_t longitude_bin;

    if (lon >= 0) { // East
    	longitude_bin = (lon * MaxEastPosition) / 180;
    } else {        // West
    	longitude_bin = (lon * MaxWestPosition) / 180;
    }
    return (uint32_t) (longitude_bin >> 8);
}

float gps_unpack_latitude_i24_to_f(const uint32_t latitude_i24) {
    int32_t latitude_bin = (int32_t)(latitude_i24 << 8);
    float lat = latitude_bin * 90.0;
    if (lat > 0) {
    	return lat / MaxNorthPosition;
    } else {
    	return lat / MaxSouthPosition;
    }
}

float gps_unpack_longitude_i24_to_f(const uint32_t longitude_i24) {
    int32_t longitude_bin = (int32_t)(longitude_i24 << 8);
    float lon = longitude_bin * 90.0;
    if (lon > 0) {
    	return lon / MaxNorthPosition;
    } else {
    	return lon / MaxSouthPosition;
    }
}


#if 0

static uint32_t gps_latitude_to_i24(const float lat) {
	int32_t field_latitude; /* 3 bytes, derived from reference latitude */
    field_latitude = (int32_t)((lat / 90.0) * (double)(1<<23));
    if (field_latitude > (int32_t)0x007FFFFF) {
        field_latitude = (int32_t)0x007FFFFF; /* +90 N is represented as 89.99999 N */
    } else if (field_latitude < (int32_t)0xFF800000) {
        field_latitude = (int32_t)0xFF800000;
    }
    return field_latitude;
}

static uint32_t gps_longitude_to_i24(const float lat) {
    int32_t field_longitude; /* 3 bytes, derived from reference longitude */
    field_longitude = (int32_t)((lon / 180.0) * (double)(1<<23));
    if (field_longitude > (int32_t)0x007FFFFF) {
    	return (uint32_t)0x007FFFFF; /* +180 E is represented as 179.99999 E */
    } else if (field_longitude < (int32_t)0xFF800000) {
    	return (uint32_t)0xFF800000;
    }
}
#endif

#if 0
static void gps_coord_to_i24(const float lat, const float lon, uint8_t* payload) {

	const uint32_t field_latitude = gps_latitude_to_i24(lat); /* 3 bytes, derived from reference latitude */
	const uint32_t field_longitude = gps_longitude_to_i24(lon); /* 3 bytes, derived from reference longitude */

    payload[0] = 0xFF &  field_latitude;
    payload[1] = 0xFF & (field_latitude >>  8);
    payload[2] = 0xFF & (field_latitude >> 16);
    payload[3] = 0xFF &  field_longitude;
    payload[4] = 0xFF & (field_longitude >>  8);
    payload[5] = 0xFF & (field_longitude >> 16);
}
#endif
