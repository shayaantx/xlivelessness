# XLLN - xlivelessness
Games for Windows Livelessness. A complete Games For Windows Live (GFWL) rewrite.

## Purpose
With GFWL being slowly phased out and many online features disappearing across the range of titles using that library, the idea is to rewrite or stub all aspects of it. Doing so also improves one's ability to reverse engineer (and improve) the titles themselves due to the anti-debugging measures embedded within the original module. Title specific modifications however do not belong here. The original implementation of this rewrite was targeted at Halo 2 which is why not all exports have been rewritten yet/in early versions.

## Title Specific Modifications
On process attach, this module searches for and loads Dynamic-Link Libraries (.DLL) in the "./xlln/modules/" folder from the working directory. If the module to be loaded is not for the title currently loading (or finishes its patches and doesn't need to stay loaded?) then it should "return FALSE;" from the DllMain entry point.

## Contributions
You may make contributions via pull requests. It is recommended to talk to Glitchy Scripts first about what it is you are planning as it may conflict with future plans.
