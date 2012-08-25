extern byte *position;

void read_boot_record(void);
void read_fats_and_root_directory_entry();
dword find_cluster(byte *, byte);
int read_cluster(word);
