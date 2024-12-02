//
// AioP13.cpp
//
// An implementation of Plan13 in C++ by Mark VandeWettering
//
// Plan13 is an algorithm for satellite orbit prediction first formulated
// by James Miller G3RUH.  I learned about it when I saw it was the basis 
// of the PIC based antenna rotator project designed by G6LVB.
//
// http://www.g6lvb.com/Articles/LVBTracker2/index.htm
//
// I ported the algorithm to Python, and it was my primary means of orbit
// prediction for a couple of years while I operated the "Easy Sats" with 
// a dual band hand held and an Arrow antenna.
//
// I've long wanted to redo the work in C++ so that I could port the code
// to smaller processors including the Atmel AVR chips.  Bruce Robertson,
// VE9QRP started the qrpTracker project to fulfill many of the same goals,
// but I thought that the code could be made more compact and more modular,
// and could serve not just the embedded targets but could be of more
// use for more general applications.  And, I like the BSD License a bit
// better too.
//
// So, here it is!
//
// =====================================================================
//
// dl9sec@gmx.net (02..11/2021):
//
// The original Plan13 BBC Basic source code can be found at:
//
// https://www.amsat.org/articles/g3ruh/111.html
//
// Published as "Donationware" in favour of AMSAT-UK, LONDON, E12 5EQ
// and the AO-13 Amateur Satellite Program.
//
// Changes:
// - Refactoring for seamless use with Arduino.
// - Renamed class DateTime to P13DateTime because of potential conflict
//   with RTClib, which uses the same class name.
// - Renamed all classes to P13... for consistency and clarification of
//   affiliation.
// - Some code beautification.
// - Changed output of method "ascii" to ISO date format
// - Added helper function for converting lat/lon coordinates to rectangular
//   map coordinates
// - Used all double
// - Used PI instead of M_PI
// - Used degrees() and radians() instead of DEGREES() and RADIANS()
// - Inserted explicit casts
// - Added a method "footprint" to class P13Satellite
// - Added a method "doppler" to calculate down- and uplink frequencies
// - Added a method "footprint" to class P13Sun
// - Added implementation suggestion for P13Sun:elaz() by Uwe Nagel
// - Renamed the whole stuff from ArduinoP13 to AioP13 to follow the
//   Arduino library specifications for naming of libraries.
// - Rework of all the variable names because of conflicts.
//   All variables got a qualifier to get more or less unique names:
//   "g_": global variables
//   "c_": class public variables
//   "cp_": class private variables
//   "p_": parameter variable
//   "l_": local variable
//   The next letter gives a hint about the data type (e.g. "d": double,
//   "i": integer, "cc": constant character, "vec": Vec3 vector, ...).
//   In some cases this is very ugly, so all the variables should be renamed
//   to speaking and useful names in one of the next releases.
//
// Uwe Nagel (09/2021)
//
// - Implementation of P13Sun:elaz()
//
// Mark Smith @SmittyHalibut (12/2021)
//
// - Added some helper constants, cleaned up compiler warnings.
// - Copy Observer and Satellite name strings instead of assuming 
//   the source is always there.
// - Added `dopplerOffset()`
//
//----------------------------------------------------------------------

#include "AioP13.h"


//----------------------------------------------------------------------
//  _  _     _                  __              _   _             
// | || |___| |_ __  ___ _ _   / _|_  _ _ _  __| |_(_)___ _ _  ___
// | __ / -_) | '_ \/ -_) '_| |  _| || | ' \/ _|  _| / _ \ ' \(_-<
// |_||_\___|_| .__/\___|_|   |_|  \_,_|_||_\__|\__|_\___/_||_/__/
//            |_|                                                 
//
//----------------------------------------------------------------------

// Convert date to day-number
static long fnday(int p_iy, int p_im, int p_id) {
    
    if ( p_im < 3 )
    {
        p_im += 12;
        p_iy--;
    }
    
    return ((long)((double)p_iy * g_scdYM) + (long)((double)(p_im + 1) * 30.6) + (long)(p_id - 428));
}


// Convert day-number to date; valid 1900 Mar 01 - 2100 Feb 28
static void fndate(int &p_iy, int &p_im, int &p_id, long p_idt) {
    
    p_idt += 428L;
    p_iy   = (int)(((double)p_idt - 122.1) / g_scdYM);
    p_idt -= (long)((double)p_iy * g_scdYM);
    p_im   = (int)((double)p_idt / 30.61);
    p_idt -= (long)((double)p_im * 30.6);
    p_im--;
    
    if ( p_im > 12 )
    {
        p_im -= 12;
        p_iy++;
    }
    
    p_id = (int)p_idt;
}


// Cut characters from c with start i0 to i1-1, append string
// delimiter and convert to double
static double getdouble(const char *p_ccc, int p_ii0, int p_ii1) {
    
    char l_acbuf[20];
    int  l_ii;
    
    for ( l_ii = 0; (p_ii0 + l_ii) < p_ii1; l_ii++ ) {
        l_acbuf[l_ii] = p_ccc[p_ii0 + l_ii];
    }
    
    l_acbuf[l_ii] = '\0';
    
    return (strtod(l_acbuf, NULL));
}


// Cut characters from c with start i0 to i1-1, append string
// delimiter and convert to long
static long getlong(const char *p_ccc, int p_ii0, int p_ii1) {
    
    char l_acbuf[20];
    int  l_ii;
    
    for ( l_ii = 0; (p_ii0 + l_ii) < p_ii1; l_ii++ ) {
        l_acbuf[l_ii] = p_ccc[p_ii0 + l_ii];
    }
    
    l_acbuf[l_ii] = '\0';
    
    return (atol(l_acbuf));
}


// Converts latitude (Breitengrad) -90..90° / longitude (Laengengrad) -180..180°
// to x/y-coordinates of a map with maxamimum dimension MapMaxX * MapMaxY
void latlon2xy(int &p_ix, int &p_iy, double p_dlat, double p_dlon, const int p_ciMapMaxX, const int p_ciMapMaxY) {
    
    p_ix = (int)((((180.0 + p_dlon) / 360.0) * (double)p_ciMapMaxX));
    p_iy = (int)(((( 90.0 - p_dlat) / 180.0) * (double)p_ciMapMaxY));
    
}


//----------------------------------------------------------------------
//     _              ___  _ _______       _      _____ _           
//  __| |__ _ ______ | _ \/ |__ /   \ __ _| |_ __|_   _(_)_ __  ___ 
// / _| / _` (_-<_-< |  _/| ||_ \ |) / _` |  _/ -_)| | | | '  \/ -_)
// \__|_\__,_/__/__/ |_|  |_|___/___/\__,_|\__\___||_| |_|_|_|_\___|
//                                                       
//----------------------------------------------------------------------

P13DateTime::P13DateTime() {
    
   c_lDN = 0L;
   c_dTN = 0.0;
}


P13DateTime::P13DateTime(const P13DateTime &p_dt) {
    
    c_lDN = p_dt.c_lDN;
    c_dTN = p_dt.c_dTN;
}


P13DateTime::P13DateTime(int p_iyear, int p_imonth, int p_iday, int p_ih, int p_im, int p_is) {

    settime(p_iyear, p_imonth, p_iday, p_ih, p_im, p_is);
}


P13DateTime::~P13DateTime() {
    
}


void P13DateTime::add(double p_ddays) {
    
    c_dTN += p_ddays;
    c_lDN += (long)c_dTN;
    c_dTN -= (long)c_dTN;
}


void P13DateTime::settime(int p_iyear, int p_imonth, int p_iday, int p_ih, int p_im, int p_is) {
    
    c_lDN = fnday(p_iyear, p_imonth, p_iday);
    c_dTN = ((double)p_ih + (double)p_im / 60.0 + (double)p_is / 3600.0) / 24.0;
}


void P13DateTime::gettime(int &p_iyear, int &p_imonth, int &p_iday, int &p_ih, int &p_im, int &p_is) {
    
    double l_dtime;

    fndate(p_iyear, p_imonth, p_iday, c_lDN);
    l_dtime  = c_dTN;
    l_dtime *= 24.0;
    p_ih  = (int)l_dtime;
    l_dtime -= (double)p_ih;
    l_dtime *= 60.0;
    p_im  = (int)l_dtime;
    l_dtime -= (double)p_im ;
    l_dtime *= 60.0;
    p_is  = (int)(l_dtime);
}


void P13DateTime::ascii(char *p_cbuf) {
    
    int l_iyear, l_imon, l_iday;
    int l_ih, l_im, l_is;
    
    gettime(l_iyear, l_imon, l_iday, l_ih, l_im, l_is);
    // 2019-05-11 00:53:13
    sprintf(p_cbuf, "%4d-%02d-%02d %02d:%02d:%02d", l_iyear, l_imon, l_iday, l_ih, l_im, l_is);
}


void P13DateTime::roundup(double p_dtime) {
    
    double l_dinc;
    
    l_dinc = p_dtime - fmod(c_dTN, p_dtime);
    
    c_dTN += l_dinc;
    c_lDN += (long)c_dTN;
    c_dTN -= (long)c_dTN;
}


//----------------------------------------------------------------------
//     _              ___  _ ____ ___  _                            
//  __| |__ _ ______ | _ \/ |__ // _ \| |__ ___ ___ _ ___ _____ _ _ 
// / _| / _` (_-<_-< |  _/| ||_ \ (_) | '_ (_-</ -_) '_\ V / -_) '_|
// \__|_\__,_/__/__/ |_|  |_|___/\___/|_.__/__/\___|_|  \_/\___|_|  
//                                                      
//----------------------------------------------------------------------

// Set the observers parameters from latitude, longitude and altitude
// above sea level (m).
P13Observer::P13Observer(const char *p_ccnm, double p_dlat, double p_dlon, double p_dasl) {
    
    double l_dD, l_dCL, l_dSL, l_dCO, l_dSO;
    double l_dRx;
    double l_dRz;
    
    uint8_t len = strlen(p_ccnm)%256;
    c_ccObsName = new char[len+1];
    strncpy(c_ccObsName, p_ccnm, len);
    c_ccObsName[len] = '\0';

    c_dLA = radians(p_dlat);
    c_dLO = radians(p_dlon);
    c_dHT = p_dasl / 1000.0;

    l_dCL = cos(c_dLA);
    l_dSL = sin(c_dLA);
    l_dCO = cos(c_dLO);
    l_dSO = sin(c_dLO);

    c_vecU[0] = l_dCL * l_dCO;
    c_vecU[1] = l_dCL * l_dSO;
    c_vecU[2] = l_dSL;

    c_vecE[0] = -l_dSO;
    c_vecE[1] =  l_dCO;
    c_vecE[2] =  0.0;

    c_vecN[0] = -l_dSL * l_dCO;
    c_vecN[1] = -l_dSL * l_dSO;
    c_vecN[2] =  l_dCL;

    l_dD = sqrt(g_scdRE * g_scdRE * l_dCL * l_dCL + g_scdRP * g_scdRP * l_dSL * l_dSL);
    l_dRx = (g_scdRE * g_scdRE) / l_dD + c_dHT;
    l_dRz = (g_scdRP * g_scdRP) / l_dD + c_dHT;

    c_vecO[0] = l_dRx * c_vecU[0];
    c_vecO[1] = l_dRx * c_vecU[1];
    c_vecO[2] = l_dRz * c_vecU[2];

    c_vecV[0] = -c_vecO[1] * g_scdW0;
    c_vecV[1] =  c_vecO[0] * g_scdW0;
    c_vecV[2] =  0.0;

}


P13Observer::~P13Observer() {
    if (c_ccObsName) {
        delete c_ccObsName;
    }    
}

//----------------------------------------------------------------------
//     _              ___  _ _______       _       _ _ _ _       
//  __| |__ _ ______ | _ \/ |__ / __| __ _| |_ ___| | (_) |_ ___ 
// / _| / _` (_-<_-< |  _/| ||_ \__ \/ _` |  _/ -_) | | |  _/ -_)
// \__|_\__,_/__/__/ |_|  |_|___/___/\__,_|\__\___|_|_|_|\__\___|
//
//----------------------------------------------------------------------

P13Satellite::P13Satellite(const char *p_ccnm, const char *p_ccl1, const char *p_ccl2) {
    c_ccSatName = nullptr;
    tle(p_ccnm, p_ccl1, p_ccl2);
}

P13Satellite::~P13Satellite() {
    if (c_ccSatName) {
        delete c_ccSatName;
    }
}

// Get satellite data from the TLE

void P13Satellite::tle(const char *p_ccnm, const char *p_ccl1, const char *p_ccl2) {
    
    double l_dCI;
    
    uint8_t len = strlen(p_ccnm)%256;
    if (c_ccSatName) {
        delete c_ccSatName;
    }
    c_ccSatName = new char[len+1];
    strncpy(c_ccSatName, p_ccnm, len);
    c_ccSatName[len] = '\0';


    // Direct quantities from the orbital elements

    cp_lN  = getlong(p_ccl1,  2,  7);               // Get satellite catalog number from tle:l1:2..6
    cp_lYE = getlong(p_ccl1, 18, 20);               // Get epoch year from tle:l1:18..19
        
    if ( cp_lYE < 58 )
        cp_lYE += 2000;
    else
        cp_lYE += 1900;

    cp_dTE = getdouble(p_ccl1, 20, 32);             // Get epoch (day of the year and fractional portion of the day) from tle:l1:20..31
    cp_dM2 = 2.0 * PI * getdouble(p_ccl1, 33, 43);  // Get first time derivative of the mean motion divided by to from tle:l1:33..42

    cp_dIN = radians(getdouble(p_ccl2, 8, 16));     // Get inclination (degrees) from tle:l2:8..15
    cp_dRA = radians(getdouble(p_ccl2, 17, 25));    // Get R.A.A.N (degrees) from tle:l2:17..24
    cp_dEC = getdouble(p_ccl2, 26, 33) / 1.0E7;     // Get eccentricity from tle:l2:26..32
    cp_dWP = radians(getdouble(p_ccl2, 34, 42));    // Get argument of perigee (degrees) from tle:l2:34..41
    cp_dMA = radians(getdouble(p_ccl2, 43, 51));    // Get mean anomaly (degrees) from tle:l2:43..50
    cp_dMM = 2.0 * PI * getdouble(p_ccl2, 52, 63);  // Get mean motion from tle:l2:52..62
    cp_lRV = getlong(p_ccl2, 63, 68);               // Get Revolution number at epoch (revolutions) from tle:l2:63..67

    // Derived quantities from the orbital elements 

    // convert TE to DE and TE 
    cp_lDE = fnday(cp_lYE, 1, 0) + (long)cp_dTE;
    
    cp_dTE -= (long)cp_dTE;

    cp_dN0  = cp_dMM / 86400.0;
    cp_dA_0 = pow(g_scdGM / (cp_dN0 * cp_dN0), 1.0/3.0);
    cp_dB_0 = cp_dA_0 * sqrt(1.0 - cp_dEC * cp_dEC);
    
    cp_dPC  = g_scdRE * cp_dA_0 / (cp_dB_0 * cp_dB_0);
    cp_dPC  = 1.5 * g_scdJ2 * cp_dPC * cp_dPC * cp_dMM;
    
    l_dCI = cos(cp_dIN);
    cp_dQD = -cp_dPC * l_dCI;
    cp_dWD =  cp_dPC * (5.0 * l_dCI * l_dCI - 1.0) / 2.0;
    cp_dDC = -2.0 * cp_dM2 / (3.0 * cp_dMM);
}


void P13Satellite::predict(const P13DateTime &p_dt) {
    
    long   l_lDN;
    
    double l_dTN;
    double l_dGHAE, l_dGHAA;
    double l_dT, l_dDT, l_dKD, l_dKDP;
    double l_dM, l_dDR, l_dEA;
    double l_dDNOM, l_dC_EA, l_dS_EA;
    double l_dA, l_dB, l_dD;
    double l_dAP, l_dCW, l_dSW;
    double l_dRAAN;
    double l_dCQ, l_dSQ;
    double l_dCI, l_dSI;
    double l_dCG, l_dSG;
    
    Vec3 l_vecCX, l_vecCY, l_vecCZ;
    
    l_lDN = p_dt.c_lDN;
    l_dTN = p_dt.c_dTN;

    l_dGHAE = radians(g_scdG0) + ((double)(cp_lDE - fnday(g_scdYG, 1, 0)) + cp_dTE) * g_scdWE;    // GHA Aries, epoch

    l_dT   = (double)(l_lDN - cp_lDE) + (l_dTN - cp_dTE);    // Elapsed T since epoch, days
    l_dDT  = cp_dDC * l_dT / 2.0;                            // Linear drag terms
    l_dKD  = 1.0 + 4.0 * l_dDT;                              // -"-
    l_dKDP = 1.0 - 7.0 * l_dDT;                              // -"-
  
    l_dM   = cp_dMA + cp_dMM * l_dT * (1.0 - 3.0 * l_dDT);   // Mean anomaly at YR,TN
    l_dDR  = (long)(l_dM / (2.0 * PI));                      // Strip out whole no of revs
    l_dM  -= l_dDR * 2.0 * PI;                               // M now in range 0..2PI
    
    // Solve M = EA - EC*SIN(EA) for EA given M, by Newton's Method
    l_dEA  = l_dM;                                           // Initial solution

    do
    {
        l_dC_EA = cos(l_dEA);
        l_dS_EA = sin(l_dEA);
        l_dDNOM = 1.0 - cp_dEC * l_dC_EA;
        l_dD = (l_dEA - cp_dEC * l_dS_EA - l_dM) / l_dDNOM;  // Change to EA for better solution
        l_dEA -= l_dD ;                                      // by this amount
    }
    while (fabs(l_dD) > 1.0E-5);

    // Distances
    l_dA = cp_dA_0 * l_dKD;           
    l_dB = cp_dB_0 * l_dKD;
    cp_dRS = l_dA * l_dDNOM;

    // Calc satellite position & velocity in plane of ellipse
    c_vecS[0] = l_dA * (l_dC_EA - cp_dEC);
    c_vecS[1] = l_dB * l_dS_EA;
    
    c_vecV[0] = -l_dA * l_dS_EA / l_dDNOM * cp_dN0;
    c_vecV[1] =  l_dB * l_dC_EA / l_dDNOM * cp_dN0;

    l_dAP = cp_dWP + cp_dWD * l_dT * l_dKDP;
    l_dCW = cos(l_dAP);
    l_dSW = sin(l_dAP);
    l_dRAAN = cp_dRA + cp_dQD * l_dT * l_dKDP;
    l_dCQ = cos(l_dRAAN);
    l_dSQ = sin(l_dRAAN);

    // CX, CY, and CZ form a 3x3 matrix that converts between orbit
    // coordinates, and celestial coordinates.
    
    // Plane -> celestial coordinate transformation, [C] = [RAAN]*[IN]*[AP]
    l_dCI = cos(cp_dIN);
    l_dSI = sin(cp_dIN);    
   
    l_vecCX[0] =  l_dCW * l_dCQ - l_dSW * l_dCI * l_dSQ;
    l_vecCX[1] = -l_dSW * l_dCQ - l_dCW * l_dCI * l_dSQ;
    l_vecCX[2] =  l_dSI * l_dSQ;

    l_vecCY[0] =  l_dCW * l_dSQ + l_dSW * l_dCI * l_dCQ;
    l_vecCY[1] = -l_dSW * l_dSQ + l_dCW * l_dCI * l_dCQ;
    l_vecCY[2] = -l_dSI * l_dCQ;

    l_vecCZ[0] = l_dSW * l_dSI;
    l_vecCZ[1] = l_dCW * l_dSI;
    l_vecCZ[2] = l_dCI;

    // Compute SATellite's position vector and VELocity in
    // CELESTIAL coordinates. (Note: Sz=S[2]=0, Vz=V[2]=0)
    c_vecSAT[0] = c_vecS[0] * l_vecCX[0] + c_vecS[1] * l_vecCX[1];
    c_vecSAT[1] = c_vecS[0] * l_vecCY[0] + c_vecS[1] * l_vecCY[1];
    c_vecSAT[2] = c_vecS[0] * l_vecCZ[0] + c_vecS[1] * l_vecCZ[1];

    c_vecVEL[0] = c_vecV[0] * l_vecCX[0] + c_vecV[1] * l_vecCX[1];
    c_vecVEL[1] = c_vecV[0] * l_vecCY[0] + c_vecV[1] * l_vecCY[1];
    c_vecVEL[2] = c_vecV[0] * l_vecCZ[0] + c_vecV[1] * l_vecCZ[1];

    // Also express SAT and VEL in GEOCENTRIC coordinates:
    l_dGHAA = (l_dGHAE + g_scdWE * l_dT); // GHA Aries at elapsed time T
    l_dCG   = cos(-l_dGHAA);
    l_dSG   = sin(-l_dGHAA);

    c_vecS[0] = c_vecSAT[0] * l_dCG - c_vecSAT[1] * l_dSG;
    c_vecS[1] = c_vecSAT[0] * l_dSG + c_vecSAT[1] * l_dCG;
    c_vecS[2] = c_vecSAT[2];

    c_vecV[0] = c_vecVEL[0] * l_dCG - c_vecVEL[1]* l_dSG;
    c_vecV[1] = c_vecVEL[0] * l_dSG + c_vecVEL[1]* l_dCG;
    c_vecV[2] = c_vecVEL[2];
}


void P13Satellite::latlon(double &p_dlat, double &p_dlon) {
    
    p_dlat = degrees(asin(c_vecS[2] / cp_dRS));
    p_dlon = degrees(atan2(c_vecS[1], c_vecS[0]));
}


void P13Satellite::elaz(const P13Observer &p_obs, double &p_del, double &p_daz) {
    
    double l_dr, l_du, l_de, l_dn;
    
    Vec3 l_vecR; // Rangevec

    
    // Rangevec = Satvec - Obsvec
    l_vecR[0] = c_vecS[0] - p_obs.c_vecO[0];
    l_vecR[1] = c_vecS[1] - p_obs.c_vecO[1];
    l_vecR[2] = c_vecS[2] - p_obs.c_vecO[2];
    
    // Range magnitude
    l_dr = sqrt(l_vecR[0] * l_vecR[0] + l_vecR[1] * l_vecR[1] + l_vecR[2] * l_vecR[2]);
    
    // Normalise Range vector
    l_vecR[0] /= l_dr;
    l_vecR[1] /= l_dr;
    l_vecR[2] /= l_dr;
    
    // UP Component of unit range
    l_du = l_vecR[0] * p_obs.c_vecU[0] + l_vecR[1] * p_obs.c_vecU[1] + l_vecR[2] * p_obs.c_vecU[2];
    // EAST
    l_de = l_vecR[0] * p_obs.c_vecE[0] + l_vecR[1] * p_obs.c_vecE[1];
    //NORTH
    l_dn = l_vecR[0] * p_obs.c_vecN[0] + l_vecR[1] * p_obs.c_vecN[1] + l_vecR[2] * p_obs.c_vecN[2];
    
    // Azimuth
    p_daz = degrees(atan2(l_de, l_dn));
    
    if (p_daz < 0.0)
        p_daz += 360.0;
    
    // Elevation
    p_del = degrees(asin(l_du));
    
    // Calculate range rate needed for doppler calculation
    // Resolve Sat-Obs velocity vector along unit range vector. (VOz=obs.V[2]=0)
    cp_dRR  = (c_vecV[0] - p_obs.c_vecV[0]) * l_vecR[0] + (c_vecV[1] - p_obs.c_vecV[1]) * l_vecR[1] + c_vecV[2] * l_vecR[2];    // Range rate, km/s
    
}

// Generates the footprint for a satellite at satlat/satlon and calculates rectangular
// x/y coordinates scaled to a map with size MapMaxX/MapMaxY. The coordinates are stored
// in a two dimensional array. points[n][0] stores x and points[n][1] stores y.
// The coordinates can be concatenated with lines to create a footprint outline.
void P13Satellite::footprint(int p_aipoints[][2], int p_inumberofpoints, const int p_ciMapMaxX, const int p_ciMapMaxY, double &p_dsatlat, double &p_dsatlon) {

    int l_ii;
    
    double l_dsrad;
    double l_dcla, l_dsla, l_dclo, l_dslo;
    double l_dsra, l_dcra;
    
    double l_da, l_dx, l_dy, l_dz, l_dXfp, l_dYfp, l_dZfp;
    
        
    l_dsrad = acos(g_scdRE / cp_dRS);  // Radius of footprint circle
    l_dsra  = sin(l_dsrad);            // Sin/Cos these to save time
    l_dcra  = cos(l_dsrad);

    l_dcla  = cos(radians(p_dsatlat));
    l_dsla  = sin(radians(p_dsatlat));
    l_dclo  = cos(radians(p_dsatlon));
    l_dslo  = sin(radians(p_dsatlon));
    
    for ( l_ii = 0; l_ii < p_inumberofpoints ; l_ii++)               // "numberofpoints" points to the circle
    {
        l_da = 2.0 * PI * (double)l_ii / (double)p_inumberofpoints;  // Angle around the circle
        l_dXfp = l_dcra;                                             // Circle of points centred on Lat=0, Lon=0
        l_dYfp = l_dsra * sin(l_da);                                 // assuming Earth's radius = 1
        l_dZfp = l_dsra * cos(l_da);
        
        l_dx   = l_dXfp * l_dcla - l_dZfp * l_dsla;                  // Rotate point "up" by latitude "satlat"
        l_dy   = l_dYfp;                                             // -"-
        l_dz   = l_dXfp * l_dsla + l_dZfp * l_dcla;                  // -"-
        
        l_dXfp = l_dx * l_dclo - l_dy * l_dslo;                      // Rotate point "around" through longitude "satlon"
        l_dYfp = l_dx * l_dslo + l_dy * l_dclo;                      // -"-
        l_dZfp = l_dz;                                               // -"-
        
        // Convert point to Lat/Lon and convert/scale to a pixel map
        latlon2xy(p_aipoints[l_ii][0], p_aipoints[l_ii][1], degrees(asin(l_dZfp)), degrees(atan2(l_dYfp,l_dXfp)), p_ciMapMaxX, p_ciMapMaxY);
    }

}

// Returns the RX (dir = 0 or P13_FRX) or TX (dir = 1 or P13_FTX) frequency with doppler shift.
double P13Satellite::doppler(double p_dfreqMHz, bool p_bodir) {
    
    double l_ddopplershift = dopplerOffset(p_dfreqMHz);
    
    if (p_bodir)  // TX
    {
        p_dfreqMHz = p_dfreqMHz - l_ddopplershift;
    }
    else          // RX
    {
        p_dfreqMHz = p_dfreqMHz + l_ddopplershift;
    }

    return (p_dfreqMHz);

}

// Returns just the change in frequency, relative to the observer, due to doppler shift.
double P13Satellite::dopplerOffset(double p_dfreqMHz) {
    return -p_dfreqMHz * cp_dRR / 299792.0;    //  Speed of light is 299792.0 km/s
}


//----------------------------------------------------------------------
//     _              ___  _ _______           
//  __| |__ _ ______ | _ \/ |__ / __|_  _ _  _  
// / _| / _` (_-<_-< |  _/| ||_ \__ \ || | \| |
// \__|_\__,_/__/__/ |_|  |_|___/___/\_,_|_|\_|
//                                             
//----------------------------------------------------------------------

P13Sun::P13Sun() {
    
}


P13Sun::~P13Sun() {
    
}


void P13Sun::predict(const P13DateTime &p_dt) {
    
    long   l_lDN;
    
    double l_dTN;
    double l_dT, l_dGHAE, l_dMRSE, l_dMASE, l_dTAS;
    double l_dC, l_dS;
    
    l_lDN = p_dt.c_lDN;
    l_dTN = p_dt.c_dTN;

    l_dT    = (double)(l_lDN - fnday(g_scdYG, 1, 0)) + l_dTN;
    l_dGHAE = radians(g_scdG0) + l_dT * g_scdWE;
        
    l_dMRSE = radians(g_scdG0) + l_dT * g_scdWW + PI;
    l_dMASE = radians(g_scdMAS0 + l_dT * g_scdMASD);
    l_dTAS  = l_dMRSE + g_scdEQC1 * sin(l_dMASE) + g_scdEQC2 * sin(2.0 * l_dMASE);

    // Sin/Cos Sun's true anomaly
    l_dC = cos(l_dTAS);
    l_dS = sin(l_dTAS);

    // Sun unit vector - CELESTIAL coords
    c_vecSUN[0] = l_dC;
    c_vecSUN[1] = l_dS * g_scdCNS;
    c_vecSUN[2] = l_dS * g_scdSNS;
    
    // Obtain SUN unit vector in GEOCENTRIC coordinates
    l_dC = cos(-l_dGHAE); 
    l_dS = sin(-l_dGHAE); 
        
    c_vecH[0] = c_vecSUN[0] * l_dC - c_vecSUN[1] * l_dS;
    c_vecH[1] = c_vecSUN[0] * l_dS + c_vecSUN[1] * l_dC;
    c_vecH[2] = c_vecSUN[2];
}


void P13Sun::latlon(double &p_dlat, double &p_dlon) {
    
    p_dlat = degrees(asin(c_vecH[2]));
    p_dlon = degrees(atan2(c_vecH[1], c_vecH[0]));
}


void P13Sun::elaz(const P13Observer &p_obs, double &p_del, double &p_daz) {
    
    // Copyright (c) 2021 Uwe Nagel
    
    double l_dr, l_du, l_de, l_dn;
    
	Vec3 l_vecR; // Rangevec
    
    // Rangevec = Satvec - Obsvec
    l_vecR[0] = c_vecH[0] * g_scdAU - p_obs.c_vecO[0] ;
    l_vecR[1] = c_vecH[1] * g_scdAU - p_obs.c_vecO[1] ;
    l_vecR[2] = c_vecH[2] * g_scdAU - p_obs.c_vecO[2] ;
    
    // Range magnitude
    l_dr = sqrt(l_vecR[0] * l_vecR[0] + l_vecR[1] * l_vecR[1] + l_vecR[2] * l_vecR[2]);
    
    // Normalise Range vector
    l_vecR[0] /= l_dr;
    l_vecR[1] /= l_dr;
    l_vecR[2] /= l_dr;
    
    // UP Component of unit range
    l_du = l_vecR[0] * p_obs.c_vecU[0] + l_vecR[1] * p_obs.c_vecU[1] + l_vecR[2] * p_obs.c_vecU[2];
    // EAST
    l_de = l_vecR[0] * p_obs.c_vecE[0] + l_vecR[1] * p_obs.c_vecE[1];
    //NORTH
    l_dn = l_vecR[0] * p_obs.c_vecN[0] + l_vecR[1] * p_obs.c_vecN[1] + l_vecR[2] * p_obs.c_vecN[2];
    
    // Azimuth
    p_daz = degrees(atan2(l_de, l_dn));

    if (p_daz < 0.0)
        p_daz += 360.0;
    
    // Elevation
    p_del = degrees(asin(l_du));

}


// Generates the sunlight footprint at satlat/satlon and calculates rectangular
// x/y coordinates scaled to a map with size MapMaxX/MapMaxY. The coordinates are stored
// in a two dimensional array. points[n][0] stores x and points[n][1] stores y.
// The coordinates can be concatenated with lines to create a footprint outline.
//
// This is a simplified aproach with no real calculation of the distance to the sun at a
// specific time. It is assumed that the nearest and farest distance of the sun makes almost no
// difference in footprint radius, it is always almost 0.5*PI. Therefore one astronomical
// unit is used for the distance. The same algorithm is used as for the satellite footprint
// except, that RS is replaced by AU.
void P13Sun::footprint(int p_aipoints[][2], int p_inumberofpoints, const int p_ciMapMaxX, const int p_ciMapMaxY, double &p_dsunlat, double &p_dsunlon) {

    int l_ii;
    
    double l_dsrad;
    double l_dcla, l_dsla, l_dclo, l_dslo;
    double l_dsra, l_dcra;
    
    double l_da, l_dx, l_dy, l_dz, l_dXfp, l_dYfp, l_dZfp;
    
        
    l_dsrad = acos(g_scdRE / g_scdAU);  // Radius of sunlight footprint circle
    l_dsra  = sin(l_dsrad);             // Sin/Cos these to save time
    l_dcra  = cos(l_dsrad);

    l_dcla  = cos(radians(p_dsunlat));
    l_dsla  = sin(radians(p_dsunlat));
    l_dclo  = cos(radians(p_dsunlon));
    l_dslo  = sin(radians(p_dsunlon));
    
    for ( l_ii = 0; l_ii < p_inumberofpoints ; l_ii++)  // "numberofpoints" points to the circle
    {
        l_da = 2.0 * PI * (double)l_ii / (double)p_inumberofpoints;  // Angle around the circle
        l_dXfp = l_dcra;                                             // Circle of points centred on Lat=0, Lon=0
        l_dYfp = l_dsra * sin(l_da);                                 // assuming Earth's radius = 1
        l_dZfp = l_dsra * cos(l_da);
        
        l_dx   = l_dXfp * l_dcla - l_dZfp * l_dsla;                  // Rotate point "up" by latitude "sunlat"
        l_dy   = l_dYfp;                                             // -"-
        l_dz   = l_dXfp * l_dsla + l_dZfp * l_dcla;                  // -"-
        
        l_dXfp = l_dx * l_dclo - l_dy * l_dslo;                      // Rotate point "around" through longitude "sunlon"
        l_dYfp = l_dx * l_dslo + l_dy * l_dclo;                      // -"-
        l_dZfp = l_dz;                                               // -"-
        
        // Convert point to Lat/Lon and convert/scale to a pixel map
        latlon2xy(p_aipoints[l_ii][0], p_aipoints[l_ii][1], degrees(asin(l_dZfp)), degrees(atan2(l_dYfp,l_dXfp)), p_ciMapMaxX, p_ciMapMaxY);
    }

}

// P13Predict for C wrapper

P13Predict::P13Predict(const char *p_ccSatName, const char *p_ccl1, const char *p_ccl2)
	: MyTime(), MySat(p_ccSatName, p_ccl1, p_ccl2) {}


void P13Predict::predict(const int p_iyear, const int p_imonth, const int p_iday, const int p_ih, const int p_im, int p_is){
	MyTime.settime(p_iyear, p_imonth, p_iday, p_ih, p_im, p_is);
	MySat.predict((const P13DateTime &)MyTime);
}

void P13Predict::latlon(double &p_dlat, double &p_dlon) {
    MySat.latlon(p_dlat, p_dlon);
}


