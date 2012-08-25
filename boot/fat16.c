#include <boot/global.h>
#include <boot/fat16.h>
#include <boot/ide.h>

/* 메모리 공간들 */
byte *boot_record = (byte*)0x7c00;	// 부트섹터가 로드될 자리 (0x7c00)
byte *fat = (byte*)0x10000;			// FAT table이 로드될 자리 (0x10000)
byte *directory_entry;				// 디렉토리 엔트리 테이블이
									// 로드될 자리 (가변)
									
/* 부트레코드에서 읽어들인 몇몇 중요한 수치들 */
word bytes_per_sector;				// 섹터당 바이트 (512)
byte sectors_per_cluster;			// 클러스터당 섹터 (1)
word reserved_sector_count;			// 예약된 섹터수 (1)
byte number_of_fats;				// FAT 테이블의 수 (2)
word root_directory_entry_count;	// 루트 디렉토리 엔트리 테이블이 가지는
									// 엔트리 최대 개수 (512)
word fat_size;						// FAT 테이블의 크기 (16)

/* 부트레코드로부터 읽은 수치들로 계산한 디스크상의 각 섹션 오프셋 */
dword fats_offset;					// 디스크상의 FAT 영역 오프셋
dword root_directory_entry_offset;	// 디스크상의 RDE 영역 오프셋
dword data_area_offset;				// 디스크상의 데이터영역 오프셋

/* 클러스터가 메모리에 적재된 위치를 표시하기 위한 변수. 모듈화, 직교성에 위배되므로 나중에 필히 고쳐야 한다. */
byte *position;

/* 클러스터를 저장하기 위한 임시변수 */
dword cluster = 0;

/* ******************** read_boot_record 함수 **********
   부트레코드를 읽어서 정보를 얻어오는 루틴 
*/
void read_boot_record() {
	/* 디스크의 최초 512바이트를 읽어보고, 섹터크기가 512바이트 이상일 경우 한섹터만큼을 채워서 더 읽음 */
	read_sector_lba(LBA2CHS(0,0,0,1), (unsigned short*)boot_record);

	bytes_per_sector = *(word*)(boot_record+11);

	/* 부트레코드 상의 특정 위치에 기록된 중요 정보들을 얻음 */
	sectors_per_cluster = *(boot_record+13);
	reserved_sector_count = *(word*)(boot_record+14);
	number_of_fats = *(boot_record+16);
	root_directory_entry_count = *(word*)(boot_record+17);
	fat_size = *(word*)(boot_record+22);

	/* 위에서 얻은 정보를 바탕으로 각 섹션의 오프셋 계산 */
	fats_offset = reserved_sector_count;
	
	root_directory_entry_offset = reserved_sector_count
		+ number_of_fats * fat_size;
	
	data_area_offset = reserved_sector_count
		+ number_of_fats * fat_size
		+ (root_directory_entry_count / 16);
}

/* ******************** read_fats_and_root_directory_entry() 함수 **********
   FAT 테이블과 루트 디렉토리 엔트리 테이블을 한꺼번에 읽어오는 루틴.
   0x10000에 한꺼번에 적재. 노파심이지만, FAT 테이블 개수가 많고,
   디렉토리 엔트리 개수가 많으면 메모리 한계를 넘는 문제가 생길 수 
   있지 않을까.
*/
void read_fats_and_root_directory_entry() {
	/* 디렉토리 엔트리의 위치를 잡아줌 */
	directory_entry = fat + fat_size * bytes_per_sector;

	/* 한꺼번에 읽는다 */
	read_sector_lba(
		LBA2CHS(0,0,fats_offset+(number_of_fats-1)*fat_size, fat_size+root_directory_entry_count/16), 
		(unsigned short*)fat);
}

/* ******************** find_cluster() 함수 **********
   디렉토리 엔트리로부터 특정 속성의 이름을 가진 노드를 찾아,
   해당 노드의 내용을 담고 있는 시작 클러스터 번호를 리턴한다.
*/
dword find_cluster(byte *nodename, byte attribute) {
	byte *temp = directory_entry;
	dword ret_cluster = 0;
	int i, j;

	for (i =0; i < root_directory_entry_count; i++) {
		if (*temp == 0xE5)							// 빈 노드
			temp += 32;
		else if (*temp == 0xF5 || *temp == 0x0)		// 이 뒤로는 다 빈 노드
			break;
		else {
			for(j = 0; j < 11 && temp[j] == nodename[j]; j++);
			
			if (j != 11) {							// 이름이 다르다
				temp += 32;
				continue;
			}
			
			if (temp[j] != attribute)				// 속성이 다르다
				continue;

			ret_cluster = (dword)(temp[21]<<24) | (dword)(temp[20]<<16) | (dword)(temp[27]<<8) | (dword)(temp[26]);	// 클러스터 계산
			break;
		}
	}
	return ret_cluster;
}

/* ******************** next_cluster() 함수 **********
   FAT 테이블에서 링크된 다음 클러스터 번호를 구한다.
   FAT 테이블은 버전에 따라서 클러스터 번호를 저장하는 비트수가 다른데,
   FAT12는 12비트, FAT16은 16비트, FAT32는 32비트를 사용한다.
   각 FAT 엔트리는 해당하는 클러스터 번호의 데이터 영역과 1:1 대응한다.
*/
word next_cluster(word cluster) {
	return *((word*)fat + cluster);
}

/* ******************** cluster_offset() 함수 **********
   클러스터 번호에 대응하는 데이터 영역의 주소를 구한다.
   클러스터 0, 1은 별도의 정보를 갖고 있으므로,
   클러스터 2번부터 데이터 영역과 대응하게 되기에 계산시 2를  빼준다.
*/
dword cluster_offset(dword cluster) {
	return data_area_offset + (cluster-2) * sectors_per_cluster;
}

/* ******************** read_cluster() 함수 **********
   길이가 짧아서 행복하다.. ㅜㅜ 클러스터 번호를 넘겨받으면,
   해당 클러스터로 시작하여 링크되는 모든 클러스터를 모두 메모리에
   적재한다. 적재되는 위치는 directory_entry이다.
*/
int read_cluster(word starting_cluster) {
	int i = 0;
	do {
		read_sector_lba(
			LBA2CHS(0,0,cluster_offset(starting_cluster),sectors_per_cluster), 
			(unsigned short *)(directory_entry + i*sectors_per_cluster*bytes_per_sector));
		starting_cluster = next_cluster(starting_cluster);
		i++;
	} while(starting_cluster < 0xffff);
	position = directory_entry;
	return i * sectors_per_cluster;
}
