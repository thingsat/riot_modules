- git clone https://github.com/Lora-net/sx1302_hal.git; cd sx1302_hal;git checkout 6291e62ef9a161ae41037a0df1a86e9943a907b9
- 
set(COMPONENT_ADD_INCLUDEDIRS port sx1302_hal/libloragw/inc)

set(libloragw_src_dir sx1302_hal/libloragw/src)

set(COMPONENT_EXTRA_INCLUDES    ${libloragw_src_dir}/cal_fw.var
                                ${libloragw_src_dir}/arb_fw.var
                            )

set(COMPONENT_SRCS  # ${libloragw_src_dir}/loragw_aux.c #need port
                    ${libloragw_src_dir}/loragw_cal.c
                    # ${libloragw_src_dir}/loragw_debug.c #need port
                    # ${libloragw_src_dir}/loragw_gps.c #need port
                    ${libloragw_src_dir}/loragw_hal.c
                    # ${libloragw_src_dir}/loragw_i2c.c #need port
                    ${libloragw_src_dir}/loragw_reg.c
                    # ${libloragw_src_dir}/loragw_spi.c #need port
                    ${libloragw_src_dir}/loragw_stts751.c
                    # ${libloragw_src_dir}/loragw_sx125x.c #need port
                    # ${libloragw_src_dir}/loragw_sx1250.c #need port
                    ${libloragw_src_dir}/loragw_sx1302_rx.c
                    ${libloragw_src_dir}/loragw_sx1302_timestamp.c
                    ${libloragw_src_dir}/loragw_sx1302.c    
                    port/esp32_loragw_aux.c
                    port/esp32_loragw_spi.c
                    port/esp32_loragw_i2c.c
                    port/esp32_loragw_debug.c
                    port/esp32_loragw_sx125x.c
                    port/esp32_loragw_sx1250.c
                    )

set(COMPONENT_REQUIRES log)

register_component()
- 




## Migration RIOT


### ESP

```
    spi_transaction_t t = {
        .flags = 0,
        .length = 8 * command_size, //transaction length in bits
        .tx_buffer = out_buf,
        .rx_buffer = in_buf,
    };

    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;

    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
    spi_device_transmit(devHandle, &t);
    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
```


### RIOT

```
#include "periph/spi.h"

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
```


```
[donsez@MacBook-Pro-de-Didier sx1302]> git grep -n spi_device_
README.md:54:    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;
README.md:57:    spi_device_transmit(devHandle, &t);
README.md:73:[donsez@MacBook-Pro-de-Didier riot-sx1302-stratagem]> git grep -n spi_device_transmit
README.md:74:drivers/sx1302/README.md:57:    spi_device_transmit(devHandle, &t);
README.md:75:drivers/sx1302/esp32_loragw_spi.c:154:    spi_device_transmit(devHandle, &t);
README.md:76:drivers/sx1302/esp32_loragw_spi.c:185:    spi_device_transmit(devHandle, &t);
README.md:77:drivers/sx1302/esp32_loragw_spi.c:234:            spi_device_transmit(devHandle, &t);
README.md:78:drivers/sx1302/esp32_loragw_spi.c:250:        spi_device_transmit(devHandle, &t);
README.md:79:drivers/sx1302/esp32_loragw_spi.c:302:            spi_device_transmit(devHandle, &t);
README.md:80:drivers/sx1302/esp32_loragw_spi.c:318:        spi_device_transmit(devHandle, &t);
README.md:81:drivers/sx1302/esp32_loragw_sx1250.c:101:    spi_device_transmit(*(spi_device_handle_t *)lgw_spi_target, &t);
README.md:82:drivers/sx1302/esp32_loragw_sx1250.c:144:    spi_device_transmit(*(spi_device_handle_t *)lgw_spi_target, &t);
README.md:83:drivers/sx1302/esp32_loragw_sx125x.c:150:    spi_device_transmit(*(spi_device_handle_t *)spi_target, &t);
README.md:84:drivers/sx1302/esp32_loragw_sx125x.c:177:    spi_device_transmit(*(spi_device_handle_t *)spi_target, &t);
esp32_loragw_spi.c:35:// static spi_device_handle_t _sxSpiHandle;
esp32_loragw_spi.c:83:    spi_device_handle_t *devHandlePtr = malloc(sizeof(spi_device_handle_t));
esp32_loragw_spi.c:94:    spi_device_interface_config_t dev = {
esp32_loragw_spi.c:113:    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;
esp32_loragw_spi.c:120:    // spi_device_handle_t *dev = (spi_device_handle_t *)(spi_target);
esp32_loragw_spi.c:123:    free((spi_device_handle_t *)spi_target);
esp32_loragw_spi.c:151:    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;
esp32_loragw_spi.c:154:    spi_device_transmit(devHandle, &t);
esp32_loragw_spi.c:182:    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;
esp32_loragw_spi.c:185:    spi_device_transmit(devHandle, &t);
esp32_loragw_spi.c:213:    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;
esp32_loragw_spi.c:234:            spi_device_transmit(devHandle, &t);
esp32_loragw_spi.c:250:        spi_device_transmit(devHandle, &t);
esp32_loragw_spi.c:281:    spi_device_handle_t devHandle = *(spi_device_handle_t *)spi_target;
esp32_loragw_spi.c:302:            spi_device_transmit(devHandle, &t);
esp32_loragw_spi.c:318:        spi_device_transmit(devHandle, &t);
esp32_loragw_sx1250.c:101:    spi_device_transmit(*(spi_device_handle_t *)lgw_spi_target, &t);
esp32_loragw_sx1250.c:144:    spi_device_transmit(*(spi_device_handle_t *)lgw_spi_target, &t);
esp32_loragw_sx125x.c:150:    spi_device_transmit(*(spi_device_handle_t *)spi_target, &t);
esp32_loragw_sx125x.c:177:    spi_device_transmit(*(spi_device_handle_t *)spi_target, &t);
```

```
[donsez@MacBook-Pro-de-Didier driver_sx1302]> cd ../../drivers/sx1302/
[donsez@MacBook-Pro-de-Didier sx1302]> git grep -n gpio_
Makefile.dep:2:FEATURES_REQUIRED += periph_gpio_irq
Makefile.dep:4:FEATURES_OPTIONAL += periph_spi_gpio_mode
README.md:56:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
README.md:58:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_spi.c:90:    gpio_pad_select_gpio(CONFIG_SX1302_CS_GPIO);
esp32_loragw_spi.c:91:    gpio_set_direction(CONFIG_SX1302_CS_GPIO, GPIO_MODE_OUTPUT);
esp32_loragw_spi.c:92:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_spi.c:125:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_spi.c:153:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_spi.c:155:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_spi.c:184:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_spi.c:186:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_spi.c:215:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_spi.c:253:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_spi.c:283:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_spi.c:321:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_sx1250.c:100:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_sx1250.c:102:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_sx1250.c:143:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_sx1250.c:145:    gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_sx125x.c:149:    // gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_sx125x.c:151:    // gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
esp32_loragw_sx125x.c:176:    // gpio_set_level(CONFIG_SX1302_CS_GPIO, 0);
esp32_loragw_sx125x.c:178:    // gpio_set_level(CONFIG_SX1302_CS_GPIO, 1);
loragw_sx1302.c:954:    int32_t gpio_sel = MCU_AGC;
loragw_sx1302.c:957:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_0_SELECTION, gpio_sel);
loragw_sx1302.c:958:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_1_SELECTION, gpio_sel);
loragw_sx1302.c:959:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_2_SELECTION, gpio_sel);
loragw_sx1302.c:960:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_3_SELECTION, gpio_sel);
loragw_sx1302.c:961:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_4_SELECTION, gpio_sel);
loragw_sx1302.c:962:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_5_SELECTION, gpio_sel);
loragw_sx1302.c:963:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_6_SELECTION, gpio_sel);
loragw_sx1302.c:964:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_7_SELECTION, gpio_sel);
loragw_sx1302.c:1359:    int32_t gpio_sel = MCU_ARB;
loragw_sx1302.c:1362:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_0_SELECTION, gpio_sel);
loragw_sx1302.c:1363:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_1_SELECTION, gpio_sel);
loragw_sx1302.c:1364:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_2_SELECTION, gpio_sel);
loragw_sx1302.c:1365:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_3_SELECTION, gpio_sel);
loragw_sx1302.c:1366:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_4_SELECTION, gpio_sel);
loragw_sx1302.c:1367:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_5_SELECTION, gpio_sel);
loragw_sx1302.c:1368:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_6_SELECTION, gpio_sel);
loragw_sx1302.c:1369:    lgw_reg_w(SX1302_REG_GPIO_GPIO_SEL_7_SELECTION, gpio_sel);
```
