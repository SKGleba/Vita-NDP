# Vita-NDP
Device/Partition manager for PSP2

![ref0](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211437.jpg)

# Introduction:
NDP is a WIP device/partition manager for Playstation Vita.
Its meant to be used by advanced users.

# Usage:
NDP offers lots of different functions/switches/modes.

Main switches:

- "MODE:" shows RW lock status, it can be changed only once per session with a SQUARE + TRIANGLE combo.
  - RR&R - Read Only mode (for backing up/dumping devices).
  - RX&W - Full access mode (restoring/flashing devices is now allowed).
  
- "BUFSZ:" shows current size of the buffer used for dumping. Can be changed with a TRIANGE + <- or -> combo.
  - 0x2000
  - 0x4000
  - 0x8000
  
- "DEV:" shows current "master" device. Can be changed with a CIRCLE + <- or -> combo.
  - INT - Internal EMMC "NAND".
  
  ![ref1](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211327.jpg)
  
  - GCD - Game Card.
  
  ![ref2](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211330.jpg)
  
  - MCD - Memory Card.
  
  ![ref3](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211333.jpg)
  
  - CUSTOM - Advanced menu.
  
  ![ref4](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211338.jpg)
  
- "LOC:" shows current base partition (Backups location, logs etc). Can be changed with a SQUARE + <- or -> combo.
  - ux0
  - ur0
  - uma0
  - imc0
  - xmc0
  
- "LIST MODE:" - only for DEV:INT, allows you to switch between SOFTWARE and FIRMWARE partition list. Can be changed with LTRIGGER.
  - SOFTWARE - vs0, vd0, tm0, ur0, sa0, pd0 and "Enable RW access to all partitions".
  - FIRMWARE - "Perform a factory backup", active/inactive os0, active/inactive slb2, idstorage, MBR.
  
- "OPERATION MODE:" - Allows you to switch between BACKUP and RESTORE mode, not available for DEV:CUSTOM
  - BACKUP.
  - RESTORE (MODE: must be set to RX&W).
  
  
Functions:

- DUMP - Dumps the selected device/partition to "/ndp/%s.img" file in the selected base partition (LOC:).

- RESTORE - Flashes the selected device/partition using a image found in "/ndp_f/%s.img" in the selected base partition (LOC:).

- FULL DEVICE DUMP - Dumps current "master" device (DEV:) to "/ndp/%s.img" file in the selected base partition (LOC:).

- "Enable RW access to all partitions" - remounts all partitions/devices as RW.

- "Perform a factory backup" - tool used to quickly dump factory os's files. It dumps inactive os0, inactive slb2 and idstorage to "/ndp/%s.img" file in the selected base partition (LOC:).

- "Check if the device exists" - opens a dialog where you can enter the device/partition's BLK name (i.e: mcd-lp-act-entire), and NDP will check if it exists, and return the result.

- "Dump one block" - opens a dialog where you can enter the device/partition's BLK name (i.e: mcd-lp-act-entire), and NDP will check if the device exists, if yes - it will dump one block (0x200) to "/ndp/%s.x" file in the selected base partition (LOC:).

- "Dump MBR" - opens a dialog where you can enter the device/partition's BLK name (i.e: mcd-lp-act-entire), and NDP will check if the device exists, if yes - it will dump 0x40000 to "/ndp/%s.x" file in the selected base partition (LOC:).

- "Dump device (MASTER/FAT16/EXFAT)" - opens a dialog where you can enter the device/partition's BLK name (i.e: mcd-lp-act-entire), and NDP will check if the device exists, if yes - it will dump the device to "/ndp/%s.x" file in the selected base partition (LOC:). Size will be taken from the device's MBR/PBR.

![ref5](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211406.jpg)
![ref6](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211419.jpg)

- "Bruteforce device names" - runs a script which generates all possible partition blk names, and checks if they exists. The existing devices names are logged to "/ndp/devices.log" file in the selected base partition (LOC:).

![ref7](https://github.com/SKGleba/Vita-NDP/raw/master/pics/2018-08-22-211343.jpg)
