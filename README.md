umr
===

This is a fork of Unreal Media Ripper (UMR) v0.3 by Andy Ward.

The code is being fixed+updated for new compilers / platforms.

Changes since the original v0.3:
--------------------------------
- Fixes for crashes with files that have big name/export/import tables.
- Fixes for crashes when run to process multiple files.
- Fixes for endless loops with certain upkg files.
- Support for 64 bit systems.
- Support for big endian systems.
- Support for version 63, 65, 69, 71, 72, 73, 79, 80, 81, 83, and 85
  music and sound files.
- support for MP2 format music and sound extraction from v75/76 files
  (Harry Potter and the Philosopher's Stone.)
- Several code cleanups.

Supported UMX files:
--------------------
- UMX music from Unreal, Return to NaPali, Unreal Tournament, DeusEx,
  Tactical Ops, and Mobile Forces are extracted successfully.
- Harry Potter and the Philosopher's Stone mpeg layer II format music
  extracts successfully.

Supported UAX files:
--------------------
- UAX sound packages from Unreal, Return to NaPali, Unreal Tournament,
  DeusEx, Klingon Honor Guard, Wheel of Time, DS9: The Fallen, Undying,
  Nerf Arena Blast, Rune, Tactical Ops, Mobile Forces, X-COM: Enforcer,
  and Adventure Pinball: Forgotten Island  are extracted successfully.
- Harry Potter and the Philosopher's Stone mpeg layer II format sound
  extracts successfully.
- XA-compressed sounds of Harry Potter and the Chamber of Secrets are
  dumped from the packages. (experimental yet.)

Known problems:
---------------
- Kran32.umx from Unreal reports two exports with names "Kran32" and
  "kran3", but both point to the same music.  Not a UMR bug, just an
  issue with the upkg in question.
- SpaceMarines.umx and Starseek.umx from Return to NaPali report their
  data as "s3m" (Scream Tracker 3), whereas the actual music format is
  "it" (Impulse Tracker).  Not a UMR bug, just an issue with the upkgs
  in question.

