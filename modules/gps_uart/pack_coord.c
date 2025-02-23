
#if 0
static void pack_coord(const float lat, const float lon, uint8_t* payload) {

	int32_t field_latitude; /* 3 bytes, derived from reference latitude */
    field_latitude = (int32_t)((lat / 90.0) * (double)(1<<23));
    if (field_latitude > (int32_t)0x007FFFFF) {
        field_latitude = (int32_t)0x007FFFFF; /* +90 N is represented as 89.99999 N */
    } else if (field_latitude < (int32_t)0xFF800000) {
        field_latitude = (int32_t)0xFF800000;
    }

    int32_t field_longitude; /* 3 bytes, derived from reference longitude */
    field_longitude = (int32_t)((lon / 180.0) * (double)(1<<23));
    if (field_longitude > (int32_t)0x007FFFFF) {
        field_longitude = (int32_t)0x007FFFFF; /* +180 E is represented as 179.99999 E */
    } else if (field_longitude < (int32_t)0xFF800000) {
        field_longitude = (int32_t)0xFF800000;
    }

    payload[0] = 0xFF &  field_latitude;
    payload[1] = 0xFF & (field_latitude >>  8);
    payload[2] = 0xFF & (field_latitude >> 16);
    payload[3] = 0xFF &  field_longitude;
    payload[4] = 0xFF & (field_longitude >>  8);
    payload[5] = 0xFF & (field_longitude >> 16);
}
#endif
