#define MAP_SHARED    1
#define MAP_PRIVATE   2
#define MAP_UNMAP     3

#define PROT_NONE   0x1
#define PROT_READ   0x2
#define PROT_WRITE  0x4

struct ufs_stat {
  uint32_t id, type, size;
};

struct ufs_dirent {
  uint32_t inode;
  char name[28];
} __attribute__((packed));