# Common Alert Message Format (CAMF) for Emergency Warning Satellite Service (EWSS)

This module provides [C headers](build/include) for interpreting the [Common Alert Message Format](https://www.gsc-europa.eu/sites/default/files/sites/all/files/EWSS-CAMF_v1.0.pdf
) of the Emergency Warning Satellite Service

## The Common Alert Message Format (CAMF)

The Common Alert Message Format (CAMF) is based on 122 bits, to be encapsulated in the signals of
the satellite navigation systems. This size is driven by design constraints applying to some satellite
constellations, where only a limited number of bits is available for messaging. The CAMF has been
developed with these constraints. However, if satellite navigation providers have more space in their
signals (i.e. spare bits), it is still possible to use these extra bits for extending the information contained
in the EWM (e.g. for adding more severity levels, or more instructions).

CAMF messages are broadcasted by the Galileo GNSS's Emergency Warning Satellite Service (EWSS).

## References

* https://www.unoosa.org/documents/pdf/icg/2023/ICG-17/icg17_wgb_20.pdf
* https://www.gsc-europa.eu/sites/default/files/sites/all/files/EWSS-CAMF_v1.0.pdf

![EWSS forest fire](https://air.imag.fr/images/a/a8/EWSS-01.jpg)
