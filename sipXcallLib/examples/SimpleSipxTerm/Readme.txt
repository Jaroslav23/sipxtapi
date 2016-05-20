SimpleSipxTerm                                          November 2007
=====================================================================

What is this
------------
This project is a very quick and ugly piece of MFC application that implements
a simple SIP Softphone based on top of SipXtapi. Up to 4 lines (accounts) can
be assigned per instance and they are showing up as separate terminals. It was
initially created with VC6.0 but for sipXtapi the converted VS2005 version has
been submitted.

Each instance will ask which local ports to use and how many accounts/lines 
there should be created. It will then use the starting DNR to up-count the 
DNR (= username) for each next terminal.
So this example requires to login to a SIP-server with DNR-alike usernames
without password authentication.

It was made do to some quick check and testing with our traditional 3rd party 
TDM-based PBX'es (with SIP-support) and trying out quickly some scenarios on 
top of sipXTapi.

Who?
----
It was contributed to Jaro by Eize Slange (EiSl1972_at_gmail_dot_com) during 
a bug-hunt in transfer scenario. While not initially meant as example, Jaro 
found it usefull to add it as example anyway.
