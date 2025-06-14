/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes CSUG -LIG
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 */

/*
 * Decode the Thingsat frame payload
 */


// Javascript functions for décoding and encoding the frame payloads
// From https://github.com/feross/buffer/blob/master/index.js

function readUInt16LE (buf, offset) {
  offset = offset >>> 0;
  return buf[offset] | (buf[offset + 1] << 8);
}

function readInt16LE (buf, offset) {
  offset = offset >>> 0;
  var val = buf[offset] | (buf[offset + 1] << 8);
  return (val & 0x8000) ? val | 0xFFFF0000 : val;
}

function readUInt24LE (buf, offset) {
  offset = offset >>> 0;

  return ((buf[offset]) |
      (buf[offset + 1] << 8)) +
      (buf[offset + 2] * 0x10000);
}

function readUInt32LE (buf, offset) {
  offset = offset >>> 0;

  return ((buf[offset]) |
      (buf[offset + 1] << 8) |
      (buf[offset + 2] << 16)) +
      (buf[offset + 3] * 0x1000000);
}

function readIntLE (buf, offset, byteLength) {
  offset = offset >>> 0;
  byteLength = byteLength >>> 0;

  var val = buf[offset];
  var mul = 1;
  var i = 0;
  while (++i < byteLength && (mul *= 0x100)) {
    val += buf[offset + i] * mul;
  }
  mul *= 0x80;

  if (val >= mul) val -= Math.pow(2, 8 * byteLength);

  return val;
}

function readInt32LE (buf, offset) {
	return readIntLE (buf, offset, 4);
}

function readInt8 (buf, offset) {
  offset = offset >>> 0;
  if (!(buf[offset] & 0x80)) return (buf[offset]);
  return ((0xff - buf[offset] + 1) * -1);
}

function readUInt8 (buf, offset) {
  offset = offset >>> 0;
  return (buf[offset]);
}

function gps_unpack_latitude_i24_to_f(latitude_i24) {
    var lat = (latitude_i24 << 8) * 90.0;
    if (lat > 0) {
    	return lat / 8388607; //MaxNorthPosition;
    } else {
    	return lat / 8388608; //MaxSouthPosition;
    }
}

function gps_unpack_longitude_i24_to_f(longitude_i24) {
	var lon = (longitude_i24 << 8) * 180.0;
    if (lon > 0) {
    	return lon / 8388607; //MaxEastPosition;
    } else {
    	return lon / 8388608; // MaxWestPosition;
    }
}

/*
*  Usage: decodeFloat32(readUInt32LE (buf, offset))
*/ 
function decodeFloat32(uint32) {
		
	// since JSON does not support NaN
	if(uint32 === 0x7FC00000) return 'NaN';
	
    var sign = (uint32 & 0x80000000) ? -1 : 1;
    var exponent = ((uint32 >> 23) & 0xFF) - 127;
    var significand = (uint32 & ~(-1 << 23));

    if (exponent == 128)
	// since JSON does not support NaN
        return sign * ((significand) ? 'NaN' : Number.POSITIVE_INFINITY);
        //return sign * ((significand) ? Number.NaN : Number.POSITIVE_INFINITY);

    if (exponent == -127) {
        if (significand == 0) return sign * 0.0;
        exponent = -126;
        significand /= (1 << 22);
    } else significand = (significand | (1 << 23)) / (1 << 23);

    var r = sign * significand * Math.pow(2, exponent);
    
    return r; 
}

function convertBytesToString(bytes) {
  var str = '';
  var len = bytes.length;

  for (var n = 0; n < len; n++) {
	  str += String.fromCharCode(parseInt(bytes[n]));
  }
  return str;
}



// ===================================================
// Decode payloads
// ---------------------------------------------------


// See lorawan_payload.h
function getPayloadType(fPort) {
	
	switch (fPort) {
	  case 0:
	    return 'mac';
	  case 1:
	    return 'sos';
	  case 2:
	    return 'diag';
	  case 3:
	    return 'repeated';
	  case 4:
	    return 'telemetry';
	  case 5:
	    return 'gnss';
	  case 6:
	    return 'stat';
	  case 9:
	    return 'tle';
	  case 10:
	    return 'dn_text';
	  case 11:
	    return 'ed25519_pubkey';
	  case 14:
	    return 'aprs';
	  case 17:
	    return 'ewss';
	  case 202:
	    return 'app_clock';
	  case 33:
	    return 'up_text';
	  case 34:
	    return 'forwarding';
	  case 35:
	    return 'action';
	  case 36:
	    return 'timing_test';
      case 100:
	    return 'plaintext';
      case 20:
	    return 'range';
      case 21:
	    return 'range1';
      case 22:
	    return 'range2';
      case 23:
	    return 'range3';
	  default:
	  	if(fPort >= 64 && fPort <= (64+32)) {
	  		return 'xor' + (fPort-64);
	  	} else {
	  		return 'unknown'
	  	}
	}
}

function decodeSos(fPort, bytes) {
	var len = bytes.length;
	
	var res = {};
	var idx = 0;
	
	if(idx+4 > len) { return res; }
	res.firmware_version = readUInt32LE(bytes,idx);
	idx += 4;

	if(idx+1 > len) { return res; }
	res.slot_id = readUInt8(bytes,idx);
	idx += 1;

	if(idx+3 > len) { return res; }
	res.uptime = readUInt24LE(bytes,idx);
	idx += 3;

	if(idx+1 > len) { return res; }
	res.error_status = readUInt8(bytes,idx);
	idx += 1;

	if(idx+1 > len) { return res; }
	res.temperature = readUInt8(bytes,idx);
	idx += 1;

	if(idx+4 > len) { return res; }
	res.roll = readInt32LE(bytes,idx);
	idx += 4;

	if(idx+4 > len) { return res; }
	res.pitch = readInt32LE(bytes,idx);
	idx += 4;

	if(idx+4 > len) { return res; }
	res.yaw = readInt32LE(bytes,idx);
	idx += 4;

	if(idx+1 > len) { return res; }
	res.last_action_endpoint_index = readUInt8(bytes,idx);
	idx += 1;

	if(idx+2 > len) { return res; }
	res.last_action_fcnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.tx_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.tx_abort_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.rx_ok_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.rx_no_crc_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.rx_bad_crc_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.rx_endpoint_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.rx_jreq_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.rx_network_cnt = readUInt16LE(bytes,idx);
	idx += 2;

	return res;
}

function decodeDiag(fPort, bytes) {

	var len = bytes.length;
	
	var res = {};
	var idx = 0;
	
	if(idx+4 > len) { return res; }
	res.firmware_version = readUInt32LE(bytes,idx);
	idx += 4;

	if(idx+1 > len) { return res; }
	res.slot_id = readUInt8(bytes,idx);
	idx += 1;

	if(idx+3 > len) { return res; }
	res.uptime = readUInt24LE(bytes,idx);
	idx += 3;

	if(idx+1 > len) { return res; }
	res.temperature = readUInt8(bytes,idx);
	idx += 1;

	if(idx+4 > len) { return res; }
	res.timestamp = readUInt32LE(bytes,idx);
	idx += 4;

	if(idx+2 > len) { return res; }
	res.seconds_before_poweroff = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+4 > len) { return res; }
	res.latitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.longitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.altitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;
    
	if(idx+4 > len) { return res; }
	res.x = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.y = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.z = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.w = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	return res;
}

function decodeRepeated(fPort, bytes) {

	var len = bytes.length;
	
	var res = {};
	var idx = 0;

    if(idx+2 > len) { return res; }
	res.experiment_id = readUInt16LE(bytes,idx);
	idx += 2;

    if(idx+1 > len) { return res; }
	res.entry_id = readUInt8(bytes,idx);
	idx += 1;
    
    if(idx+2 > len) { return res; }
	res.rx_timestamp = readUInt16LE(bytes,idx);
	idx += 2;
    
    if(idx+1 > len) { return res; }
	res.dr = readUInt8(bytes,idx);
	idx += 1;

    if(idx+1 > len) { return res; }
	res.esp = readUInt8(bytes,idx) * -1;
	idx += 1;

    if(idx+1 > len) { return res; }
	res.rfpower = readUInt8(bytes,idx);
	idx += 1;

    // TODO uint8_t frame[REPEATED_FRAME_MAX_LEN];

	if(len < 0) {
		res.frame_size = 0;
	} else {
		res.frame_size = len - idx;
	}

	return res;
}

function decodeTelemetry(fPort, bytes) {
	var len = bytes.length;
	
	var res = {};
	var idx = 0;

    if(idx+2 > len) { return res; }
	res.entry_id = readUInt16LE(bytes,idx);
	idx += 2;

    if(idx+1 > len) { return res; }
	res.entry_id = readUInt8(bytes,idx);
	idx += 1;
    
    if(idx+1 > len) { return res; }
	res.rfpower = readUInt8(bytes,idx);
	idx += 1;
    
	if(idx+4 > len) { return res; }
	res.timestamp = readUInt32LE(bytes,idx);
	idx += 4;
    
	if(idx+2 > len) { return res; }
	res.seconds_before_poweroff = readUInt16LE(bytes,idx);
	idx += 2;
    
	if(idx+1 > len) { return res; }
	res.temperature = readUInt8(bytes,idx);
	idx += 1;    

	if(idx+4 > len) { return res; }
	res.latitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.longitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.altitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;
    
	if(idx+4 > len) { return res; }
	res.x = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.y = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.z = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.w = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.roll = readInt32LE(bytes,idx);
	idx += 4;

	if(idx+4 > len) { return res; }
	res.pitch = readInt32LE(bytes,idx);
	idx += 4;

	if(idx+4 > len) { return res; }
	res.yaw = readInt32LE(bytes,idx);
	idx += 4;
	
	return res;
}

function decodeGNSS(fPort, bytes) {

// TODO SHOULD BE TESTED

	var len = bytes.length;
	
	if(len < 8) {
		return { "error": "invalid_size" };
	}

	var res = {};
	
	var idx = 0;
	
	if(idx+4 > len) { return res; }
	res.tx_uscount = readUInt32LE(bytes,idx);
	idx += 4;

	// TODO decode status --> tx mode, fix, ftime
	if(idx+1 > len) { return res; }
	res.status = readUInt8(bytes,idx);
	idx += 1;

	if(idx+1 > len) { return res; }
	res.txpower = readUInt8(bytes,idx);
	idx += 1;

	if(idx+4 > len) { return res; }
	res.latitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+4 > len) { return res; }
	res.longitude = decodeFloat32(readUInt32LE(bytes,idx));
	idx += 4;

	if(idx+2 > len) { return res; }
	res.altitude = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.seconds_since_last_fix = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+1 > len) { return res; }
	res.satellites_tracked = readUInt8(bytes,idx);
	idx += 1;    

	if(idx+1 > len) { return res; }
	res.fix_quality = readUInt8(bytes,idx);
	idx += 1;    

	if(idx+2 > len) { return res; }
	res.speed_kph = readUInt16LE(bytes,idx);
	idx += 2;

	if(idx+2 > len) { return res; }
	res.true_track_degrees = readInt16LE(bytes,idx);
	idx += 2;

	return res;
}

function decodeTle(fPort, bytes) {
// TODO SHOULD BE TESTED

	var len = bytes.length;
	
	var res = {};
	var idx = 0;

    if(idx+1 > len) { return res; }
	res.time_param_prec = readUInt8(bytes,idx) & 0x07;
	idx += 1;

	if(idx+4 > len) { return res; }
	res.timestamp = readUInt32LE(bytes,idx);
	idx += 4;

	if(idx+3 > len) { return res; }
	res.noradnumber = readUInt24LE(bytes,idx);
	idx += 3;
	
	// TO BE CONTINUED

	return res;
}

function decodeDnText(fPort, bytes) {
	return {};
}

function decodeEd25519PubKey(fPort, bytes) {
	return {};
}

function decodeAppClock(fPort, bytes) {
// TODO SHOULD BE TESTED

	var len = bytes.length;

	var res = {};
	var idx = 0;

    if(idx+1 > len) { return res; }
	res.cid0 = readUInt8(bytes,idx);
	idx += 1;

    if(idx+1 > len) { return res; }
	res.package_identifier = readUInt8(bytes,idx);
	idx += 1;

    if(idx+1 > len) { return res; }
	res.package_version = readUInt8(bytes,idx);
	idx += 1;

    if(idx+1 > len) { return res; }
	res.cid1 = readUInt8(bytes,idx);
	idx += 1;

    if(idx+1 > len) { return res; }
	res.param_prec = readUInt8(bytes,idx) & 0x03; // 0b00000011
	idx += 1;

	if(idx+4 > len) { return res; }
	res.time_to_set = readUInt32LE(bytes,idx);
	idx += 4;

    if(idx+1 > len) { return res; }
	res.cid2 = readUInt8(bytes,idx);
	idx += 1;

	if(idx+3 > len) { return res; }
	res.noradnumber = readUInt24LE(bytes,idx);
	idx += 3;
	
	if(idx+3 > len) { return res; }
	res.lat24 = readUInt24LE(bytes,idx); // Should be converted
	idx += 3;
	
	if(idx+3 > len) { return res; }
	res.lon24 = readUInt24LE(bytes,idx);  // Should be converted
	idx += 3;
	
	if(idx+3 > len) { return res; }
	res.alt24 = readUInt24LE(bytes,idx);  // Should be converted
	idx += 3;
	
	if(idx+1 > len) { return res; }
	var loc_param = readUInt8(bytes,idx);
	res.loc_param_source = loc_param >> 7;
	res.loc_param_prec = loc_param & 0x7F; //0b01111111
	idx += 1;

	return res;
}

function decodeUpText(fPort, bytes) {
	return {};
}

function decodePlainText(fPort, bytes) {
	return { "text": convertBytesToString(bytes) };
}

function decodeForwarding(fPort, bytes) {
	return {};
}

function decodeAction(fPort, bytes) {
	return {};
}

function decodeTimingTest(fPort, bytes) {
	return {};
}

function decodeRange(fPort, bytes) {

// TODO SHOULD BE TESTED

	var len = bytes.length;
	
	if(len < 10) {
		return { "error": "invalid_size" };
	}

	var res = {};
	
	var idx = 0;
	
	res.tx_uscount = readUInt32LE(bytes,idx);
	idx += 4;

	res.tx_uscount_prev = readUInt32LE(bytes,idx);
	idx += 4;

	res.ranging_status = readUInt8(bytes,idx);
	idx += 1;

	res.txpower = readUInt8(bytes,idx);
	idx += 1;

	if(idx+3 > len) { return res; }
	res.latitude = gps_unpack_latitude_i24_to_f(readUInt24LE(bytes,idx));
	idx += 3;

	if(idx+3 > len) { return res; }
	res.longitude = gps_unpack_longitude_i24_to_f(readUInt24LE(bytes,idx));
	idx += 3;

	if(idx+2 > len) { return res; }
	res.altitude = readUInt16LE(bytes,idx);
	idx += 2;

	return res;
}

function decodeStat(fPort, bytes) {

// TODO SHOULD BE TESTED

	var len = bytes.length;
	
	if(len < 10) {
		return { "error": "invalid_size" };
	}

	var res = {};
	
	var idx = 0;
	
	res.tx_uscount = readUInt32LE(bytes,idx);
	idx += 4;

	res.tx_trigcount = readUInt32LE(bytes,idx);
	idx += 4;

	res.ranging_status = readUInt8(bytes,idx);
	idx += 1;

	res.txpower = readUInt8(bytes,idx);
	idx += 1;


/*
	if(idx+3 > len) { return res; }
	res.latitude = gps_unpack_latitude_i24_to_f(readUInt24LE(bytes,idx));
	idx += 3;

	if(idx+3 > len) { return res; }
	res.longitude = gps_unpack_longitude_i24_to_f(readUInt24LE(bytes,idx));
	idx += 3;

	if(idx+2 > len) { return res; }
	res.altitude = readUInt16LE(bytes,idx);
	idx += 2;
*/
	return res;
}




function decodeXor(fPort, bytes) {
	return { ratio : fPort - 64, buf: bytes };
}

function decodeAprs(fPort, bytes) {
	return { msg: convertBytesToString(bytes) };
}

function decodeEwss(fPort, bytes) {
	return { buf: bytes };
}

function decodePayload(fPort, bytes) {
	var data = undefined;
	switch (fPort) {
	  case 1:
	    data = decodeSos(fPort, bytes);
		break;
	  case 2:
	    data = decodeDiag(fPort, bytes);
		break;
	  case 3:
	    data = decodeRepeated(fPort, bytes);
		break;
	  case 4:
	    data = decodeTelemetry(fPort, bytes);
		break;
	  case 5:
	    data = decodeGNSS(fPort, bytes);
		break;
	  case 6:
	    data = decodeStat(fPort, bytes);
		break;
	  case 9:
	    data = decodeTle(fPort, bytes);	    
		break;
	  case 10:
	    data = decodeDnText(fPort, bytes);	    
		break;
	  case 14:
	    data = decodeAprs(fPort, bytes);	    
		break;
	  case 17:
	    data = decodeEwss(fPort, bytes);	    
		break;
      case 100:
	    data = decodePlainText(fPort, bytes);
		break;
	  case 11:
	    data = decodeEd25519PubKey(fPort, bytes);	    
		break;
	  case 202:
	    data = decodeAppClock(fPort, bytes);	    
		break;
	  case 33:
	    data = decodeUpText(fPort, bytes);	    
		break;
	  case 34:
	    data = decodeForwarding(fPort, bytes);	    
		break;
	  case 35:
	    data = decodeAction(fPort, bytes);	    
		break;
	  case 36:
	    data = decodeTimingTest(fPort, bytes);
		break;
      case 20:
      case 21:
      case 22:
      case 23:
	    data = decodeRange(fPort, bytes);
	    break;
	  default:
	  	if(fPort >= 64 && fPort <= (64+32)) {
	  		data =  decodeXor(fPort, bytes);
	  	} else {
	  		data =  {};
	  	}
		break;
	}
    return data;
}

function decodeUp(bytes, fPort) {

  var decoded = decodePayload(fPort, bytes);
  var payload_type = getPayloadType(fPort);
  decoded.port = fPort;
  decoded.payload_type = payload_type;
 
  return decoded;
}


// ===================================================
// LNS adapters
// ---------------------------------------------------

/*
* Chirpstack : Decode decodes an array of bytes into an object.
*/
function Decode(fPort, bytes, variables) {
  var decoded = decodeUp(bytes, fPort);
  //decoded._variables = variables;
  return decoded;
}

/*
* Helium : decode decodes an array of bytes into an object.
*/
function Decoder(bytes, port, uplink_info) {

  var decoded = decodeUp(bytes, port);

/*
  The uplink_info variable is an OPTIONAL third parameter that provides the following:

  uplink_info = {
    type: "join",
    uuid: <UUIDv4>,
    id: <device id>,
    name: <device name>,
    dev_eui: <dev_eui>,
    app_eui: <app_eui>,
    metadata: {...},
    fcnt: <integer>,
    reported_at: <timestamp>,
    port: <integer>,
    devaddr: <devaddr>,
    hotspots: {...},
    hold_time: <integer>
  }
*/

  if (uplink_info) {
    // do something with uplink_info fields
    decoded._uplink_info = uplink_info;
  }

  // for Mapper and Cargo https://docs.helium.com/use-the-network/console/integrations/cargo/
  if(decoded.latitude && decoded.longitude && decoded.altitude) {
    decoded.payload = {
      latitude: decoded.latitude,
      longitude: decoded.longitude,
      altitude: decoded.altitude * 1000, // in meter (not in kilometer)
      speed: 16986.84 // in mph (from https://isstracker.pl/en/satelity/51087)
	  //battery	Battery Voltage
    }
  }

  return decoded;
}

/*
* SHOULD BE TESTED
* TTN v3 : decode decodes an array of bytes into an object.
* See https://www.thethingsindustries.com/docs/integrations/payload-formatters/create/
* See https://www.thethingsindustries.com/docs/integrations/payload-formatters/javascript/uplink-decoder/
*/
function decodeUplink(input) {
  return {
    data: decodeUp(input.bytes, input.fPort),
    warnings: [],
    errors: []
  };
}

