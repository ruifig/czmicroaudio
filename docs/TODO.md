* Replace all occurences of CZ_PLATFORM with the right CZ_PLATFORM_XXX 
* Remove all references to Symbian stuff. Examples:
    * CZ_PLATFORM_SYMBIAN
* Remove all the "pass Core" around bullshit. That was because of Symbian and PALM, if I remember correctly.
* Const table can now be just globals, instead of pointers inside some object (like Core)
    * Again, this was because of PALM. If I remember correctly, PALM didn't allow accessing global pointers from the sound callback
* Remove cz::io namespace (e.g: DiskFile should move to cz::microaudio or czmicromuc library
* Port Tremor support
* Use #pragma once instead of #ifdefs on all headers
* Rename all globals in a consistent way. Maybe all as g_XXX
* Use a fixed point class so we have more flexibility on picking maximum sample sizes, etc.
* Add support for TI sample compression
    * E.g: https://wiki.multimedia.cx/index.php/Impulse_Tracker#IT214_sample_compression
    * One sample I have that uses sample compression: EYE.it
* Look for "exit(EXIT_FAILURE)" calls and do something appropriate
* Volumes passed to samples and modules seem to be in different ranges.


# NOTES FOR DOCUMENTATION

* Lots of really old code still present
    * Some of it is is from the late 90s, so style might look weird.
* Some leftover comments from old code might be completely wrong since they don't apply to this port

