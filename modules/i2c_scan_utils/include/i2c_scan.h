/*
 Thingsat Mission
 Copyright (c) 2021-2024 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _THINGSAT_MODULES_I2C_SCAN_H
#define _THINGSAT_MODULES_I2C_SCAN_H

int mission_i2c_scan(const int idx);

int mission_i2c_scan_and_check(const int dev);

#endif
