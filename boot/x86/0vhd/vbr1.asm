;; VBR para a partição de boot.
;; O volume começa no setor 63 com esse VBR (boot sector do volume).
;;
;; A FAT1 fica no setor 67, a FAT2 fica no setor ??, o root dir fica 
;; no setor 559 a área de dados começa no setor 591.
;;


;;	
;;  ## VBR ##	
;;

;; unsigned char JumpInstruction[3];
db 0xEB, 0x3C, 0x90

;; unsigned char OemName[8];
;; Compatibilidade com host.
db "MSDOS5.0" 
;;db 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30

;; unsigned short BytesPerSector;
;; 0x0200
;;Bytes per Sector. The size of a hardware sector. 
;;For most disks in use in the United States, 
;;the value of this field is 512.
db 0x00, 0x02

;;  unsigned char SectorsPerCluster;
;; Sectors Per Cluster. The number of sectors in a cluster. 
;; The default cluster size for a volume depends on the volume size 
;; and the file system.
db 0x01


;; ? 
;; unsigned short NumberOfReservedSectors;
;; O número de setores reservados que ficam entre o vbr e a primeira fat.
;; Reserved Sectors. The number of sectors from the Partition Boot Sector 
;; to the start of the first file allocation table, 
;; >>>*(INCLUDING) the Partition Boot Sector. <<<
;;The minimum value is 1.  (1+3=4)
;; If the value is greater than 1, it means that the bootstrap code 
;; is too long to fit completely in the Partition Boot Sector.
db 0x04, 0x00 


;; unsigned char NumberOfFATs;
;; Number of file allocation tables (FATs). 
;; The number of copies of the file allocation table on the volume. 
;; Typically, the value of this field is 2.
db 0x02 

;; unsigned short NumberOfRootDirectoryEntries;
;; 0x0200
;; Root Entries. The total number of file name entries 
;; that can be stored in the root folder of the volume. 
;; One entry is always used as a Volume Label. 
;; Files with long filenames use up multiple entries per file. 
;; Therefore, the largest number of files in the root folder is 
;; typically 511, but you will run out of entries sooner if you 
;; use long filenames.
db 0x00, 0x02




;; ??
;; unsigned short TotalSectorCount16;
;; Small Sectors. The number of sectors on the volume if 
;; the number fits in 16 bits (65535). 
;; For volumes larger than 65536 sectors, 
;; this field has a value of 0 and the Large Sectors field is used instead.
;; 0xF8000 = 63488 setores = 31744 KB

;; ok, this is working with 3 mb of unallocated space.
;; This way we have a partition of 32mb. see: partition table in the mbr.
;; but we have 3 mb of unallocated space.

db 0xFE, 0xE3

;; for smaller partition
; db 0xFE, 0xC3
; db 0xFE, 0x93
; db 0xFE, 0x53
; db 0xFE, 0x43 
;; 0x43FE = 17406 setores  = 8703 KB.



;; ??
;; unsigned char Media;
;; media descriptor. ok.
;; Media Type. Provides information about the media being used. 
;; A value of 0xF8 indicates a hard disk.
db 0xF8

;; ??
;; unsigned short SectorsPerFAT;
;; Sectors per file allocation table (FAT). 
;;Number of sectors occupied by each of the file allocation tables 
;;on the volume. By using this information, together with the 
;;Number of FATs and Reserved Sectors, you can compute where 
;;the root folder begins. By using the number of entries in the root folder, 
;;you can also compute where the user data area of the volume begins.
db 0xF6, 0x00 




;; ??
;; unsigned short SectorsPerTrack;
;; Sectors per Track. The apparent disk geometry in use when 
;;the disk was low-level formatted.
db 0x3F, 0x00 

;; ??
;; unsigned short NumberOfHeads;
;; Number of Heads. The apparent disk geometry in use when 
;;the disk was low-level formatted.
db 0x04, 0x00

;; ??
;; unsigned long NumberOfHiddenSectors;
;; ?? Hidden Sectors. Same as the Relative Sector field in the Partition Table.

db 0x3F, 0x00, 0x00, 0x00 ;;@todo: talvez sejam os 63 escondidos.

;; ??
;; unsigned long TotalSectorCount32;
;; Large Sectors. If the Small Sectors field is zero, 
;;this field contains the total number of sectors in the volume. 
;;If Small Sectors is nonzero, this field contains zero.
db 0x00, 0x00, 0x00, 0x00 



;; unsigned char DriveNumber;
;; Physical Disk Number. This is related to the BIOS physical disk number. 
;;Floppy drives are numbered starting with 0x00 for the A disk. 
;;Physical hard disks are numbered starting with 0x80. 
;;The value is typically 0x80 for hard disks, regardless of how many 
;;physical disk drives exist, because the value is only relevant if 
;;the device is the startup disk.
db 0x80

;; ??
;; unsigned char Reserved1;
;; Current Head. Not used by the FAT file system.
;; @todo: precisa ser 0.
db 0x01

;; unsigned char ExtendedBootSignature;
;; Signature. Must be either 0x28 or 0x29 in
;; order to be recognized by Windows NT.
db 0x29

;; ??
;; #importante
;; unsigned long VolumeSerialNumber;
;; Volume Serial Number. A unique number that is created when you 
;; format the volume.
db 0xE3, 0x50, 0xB1, 0x6E




;; unsigned char VolumeLabel[11];
;; Volume Label. This field was used to store the volume label, 
;; but the volume label is now stored as special file in the root directory.
db  "GRAMADO VBR"

;; unsigned char FileSystemType[8];
;; System ID. Either FAT12 or FAT16, depending on the format of the disk.
db "FAT16   "
;db 0x31, 0x36, 0x20, 0x20
;db 0x20, 0x33, 0xC9, 0x8E

;;
;; ?? Ainda não sei se posso usar outra coisa aqui nessa área de código.
;;
;; unsigned char BootCode[448];

db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00



;; unsigned short Signature;
db 0x55, 0xAA


