# LoRaWAN utilities

Extracted from semtech-loramac pkg 

Embed its own implementation of aes.* and cmac.*




```bash
NwkSKey=cafebabe12345678cafebabe12345678
AppSKey=443e25af51d680671891ba2eba421bd2
ReceivedFrame=807856341280ff01aa792d344b87d20550

lora-packet-decode \
        --appkey $AppSKey \
        --nwkkey $NwkSKey \
        --hex $ReceivedFrame
```

