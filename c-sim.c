//Baljit Kaur
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-sim.h"

int num_of_hits = 0;
int num_of_misses = 0;
int reads = 0;
int writes = 0;
int num_of_lines = 0;
int cache_size = 0;
int block_size = 0;
int assoc = 0;
int block_sub = 0;
int set_sub = 0;
char write_policy[] = "  ";
char replacement_policy[] = "    ";
Line* cache;

int main(int argc, char *argv[]) {
	int i;
	if (argc != 7){
		printf("Not enough arguments.\n");
		return;
	}
	
	cache_size = atoi(argv[1]);
	if(!((cache_size != 0) && (cache_size & (cache_size - 1)) == 0)){
		printf("Cache size is not a power of 2 or cache size is 0.\n");
		return 1;
	}

	block_size = atoi(argv[3]);
	if(!((block_size != 0) && ((block_size & (block_size - 1)) == 0))){
		printf("Block size is not a power of 2 or cache size is 0.\n");
		return 1;
	}else{
		for(i = 1; i < block_size; i*=2){
			block_sub++;
		}
	}
	
	char *c;
	if(strcmp(argv[2], "direct") == 0){
		assoc = 1;
	}else if(strcmp(argv[2], "assoc") == 0){
		assoc = (int)(cache_size/block_size);
	}else{
		c = strtok(argv[2], ":");
		c = strtok(NULL, ":"); 
		if(c == NULL){ 
			printf("Associativity is invalid.\n"); 
			return 1; 
		} 
		assoc = atoi(c);
	}
	
	if(strcmp(argv[4], "LRU") == 0){
		strcpy(replacement_policy, "LRU ");
	}else if(strcmp(argv[4], "FIFO") == 0){
		strcpy(replacement_policy, "FIFO"); 
	}else{
		printf("Invalid Replacement Policy.\n");
		return 1;
	}
	
	if(strcmp(argv[5], "wt") == 0){
		strcpy(write_policy, "wt"); 
	}else if(strcmp(argv[5], "wb") == 0){
		strcpy(write_policy, "wb");
	}else{
		printf("Incorrect Write Policy.\n");
		return 1;
	}
	
	FILE *file = fopen(argv[6], "r");
	if(file == 0){
		printf("File not found.\n");
		return 1;
	}
	
	num_of_lines = (int)(cache_size/block_size);
	for(i = 1; i < (num_of_lines / assoc); i*=2){ 
		set_sub++; 
	} 
	 
	char ip[20]; 
	char wr[2]; 
	char address[20]; 
	char *binary;
	
	cache = create_cache(); 
	if(num_of_lines / assoc  * assoc * block_size != cache_size || cache_size == 0){ 
		printf("Invalid size, block size, or associativity.\n"); 
		return 1; 
	} 
	
	while (fscanf(file, "%s %s %s", ip, wr, address) != EOF) { 
		if(strcmp(ip, "#eof") == 0){ 
			break; 
		} 
		if (strcmp(replacement_policy, "FIFO") == 0){
			if(strcmp(write_policy, "wt") == 0){ 
				binary = get_binary_addr(address); 
				if(strcmp(wr, "W") == 0){ 
					write_to_cache(binary); 
				}else if(strcmp(wr, "R") == 0){ 
					read_from_cache(binary); 
				} 
			}else{ 
				binary = get_binary_addr(address); 
				if(strcmp(wr, "W") == 0){
					write_to_cache_wb(binary); 
				}else if(strcmp(wr, "R") == 0){ 
					read_from_cache_wb(binary); 
				} 
			}
		}	
	} 
	printf("Memory reads: %d\n", reads); 
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", num_of_hits); 
	printf("Cache misses: %d\n", num_of_misses);  
	fclose(file); 
	free_cache(); 
	return 0;
}

Line *create_cache(){
	Line *cache = malloc(sizeof(Line) * num_of_lines);
	int i;
	for(i = 0; i < num_of_lines; i++){
		cache[i].valid = 0;
		cache[i].tag = (char *)malloc(sizeof(char) * 81);
		strcpy(cache[i].tag, "-");
		cache[i].set = i / assoc;
		cache[i].recent = 0;
		cache[i].dirty = 0;
		cache[i].tag_length = 1;
	}
	return cache;
}

int find_tag_length(char addr[]){
	return strlen(addr) - set_sub - block_sub;
}

char *get_binary_addr(char addr[]){
	int i;
	char *binary = (char *)malloc(sizeof(char) * 81);
	for(i = 2; i < strlen(addr); i++){
		switch(addr[i]){
			case '0':
				strcat(binary, "0000");
				break;
			case '1':
				strcat(binary, "0001");
				break;
			case '2':
				strcat(binary, "0010");
				break;
			case '3':
				strcat(binary, "0011");
				break;
			case '4':
				strcat(binary, "0100");
				break;
			case '5':
				strcat(binary, "0101");
				break;
			case '6':
				strcat(binary, "0110");
				break;
			case '7':
				strcat(binary, "0111");
				break;
			case '8':
				strcat(binary, "1000");
				break;
			case '9':
				strcat(binary, "1001");
				break;
			case 'a':
				strcat(binary, "1010");
				break;
			case 'b':
				strcat(binary, "1011");
				break;
			case 'c':
				strcat(binary, "1100");
				break;
			case 'd':
				strcat(binary, "1101");
				break;
			case 'e':
				strcat(binary, "1110");
				break;
			case 'f':
				strcat(binary, "1111");
				break;
		}
	}
	return binary;
}

int get_index(char address[], int tag_length){ 
	int index = 0; 
	int exp = 1; 
	int i; 
	for(i = strlen(address) - 1 - block_sub; i >= tag_length; i--){ 
		if(address[i] == '1'){ 
			index += exp; 
		} 
		exp = exp<<1;
	} 
	return index; 
} 

void update_recents(int new_index, int index){
	int i;
	for(i = index * assoc; i < index; i++){
		cache[i].recent++;
	}
	cache[new_index].recent = 0;
}

void write_to_cache(char addr[]){
	int highest = 0; 
	int index = get_index(addr, find_tag_length(addr)) * assoc; 
	int highest_index = index; 
	int i; 
	writes++; 
	for(i = index; i < index + assoc; i++){ 
		if(strncmp(cache[i].tag, addr, cache[i].tag_length) == 0){ 
			if(cache[i].valid == 1){ 
				num_of_hits++; 
				strcpy(cache[i].tag, addr);
				cache[i].tag_length = find_tag_length(addr);
				update_recents(i, cache[i].set);
				return; 
			} 
		} 
	} 
	num_of_misses++; 
	reads++; 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 0){
			cache[i].valid = 1; 
			strcpy(cache[i].tag, addr); 
			cache[i].tag_length = find_tag_length(addr); 
			update_recents(i, cache[i].set); 
			return; 
		} 
	} 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].recent > highest){ 
			highest = cache[i].recent; 
			highest_index = i; 
		} 
	} 
	cache[highest_index].tag_length = find_tag_length(addr); 
	strcpy(cache[highest_index].tag, addr); 
	update_recents(highest_index, cache[highest_index].set); 
}

void read_from_cache(char addr[]){
	int index = get_index(addr, find_tag_length(addr)) * assoc; 
	int highest_index = index; 
	int highest = 0; 
	int i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 1){ 
			if(strncmp(cache[i].tag, addr, cache[i].tag_length) == 0){ 
				num_of_hits++; 
				update_recents(i, cache[i].set); 
				return; 
			} 
		} 
	} 
	num_of_misses++; 
	reads++; 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 0){ 
			cache[i].valid = 1; 
			cache[i].tag_length = find_tag_length(addr); 
			strcpy(cache[i].tag, addr); 
			update_recents(i, cache[i].set); 
			return; 
		} 
	} 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].recent > highest){ 
	    	highest = cache[i].recent; 
			highest_index = i; 
		} 
	} 
	cache[highest_index].tag_length = find_tag_length(addr); 
	strcpy(cache[highest_index].tag, addr); 
	update_recents(highest_index, cache[highest_index].set);
}

void write_to_cache_wb(char address[]){
	int index = get_index(address, find_tag_length(address)) * assoc; 
	int highest_index = index; 
	int highest = 0; 
	int i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 1){ 
			if(strncmp(cache[i].tag, address, cache[i].tag_length) == 0){ 
				num_of_hits++; 
				cache[i].tag_length = find_tag_length(address); 
				cache[i].dirty = 1; 
				update_recents(i, cache[i].set); 
				return; 
			} 
		} 
	} 
	num_of_misses++; 
	reads++;
	index = get_index(address, find_tag_length(address)) * assoc;
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 0){ 
			cache[i].valid = 1; 
			cache[i].tag_length = find_tag_length(address);
			strcpy(cache[i].tag, address); 
			cache[i].dirty = 1; 
			update_recents(i, cache[i].set); 
			return; 
		} 
	} 
	index = get_index(address, find_tag_length(address)) * assoc;
	for(i = index; i < index + assoc; i++){ 
		if(cache[i].recent > highest){ 
			highest = cache[i].recent; 
			highest_index = i; 
		} 
	} 
	if(cache[highest_index].dirty == 1){ 
		writes++; 
	} 
	cache[highest_index].tag_length = find_tag_length(address); 
	strcpy(cache[highest_index].tag, address); 
	cache[highest_index].dirty = 1; 
	update_recents(highest_index, cache[highest_index].set);
}

void read_from_cache_wb(char addr[]){
	int i;
	int index = get_index(addr, find_tag_length(addr)) * assoc; 
	int highest_index = index; 
	int highest = 0;
	for(i = index; i < index + assoc; i++){ 
		if(cache[i].valid == 1){ 
			if(strncmp(cache[i].tag, addr, cache[i].tag_length) == 0){ 
				num_of_hits++; 
				update_recents(i, cache[i].set); 
				return; 
			} 
		} 
	} 
	reads++; 
	num_of_misses++; 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 0){ 
			cache[i].valid = 1; 
			cache[i].dirty = 0; 
			cache[i].tag_length = find_tag_length(addr); 
			strcpy(cache[i].tag, addr); 
			update_recents(i, cache[i].set); 
			return; 
		} 
	} 
	index = get_index(addr, find_tag_length(addr)) * assoc;
	for(i = index; i < index + assoc; i++){ 
		if(cache[i].recent > highest){ 
			highest = cache[i].recent; 
			highest_index = i; 
		} 
	} 
	if(cache[highest_index].dirty == 1){ 
		writes++; 
	} 
	cache[highest_index].tag_length = find_tag_length(addr); 
	strcpy(cache[highest_index].tag, addr); 
	cache[highest_index].dirty = 0; 
	update_recents(highest_index, cache[highest_index].set);
}

void write_to_cache_LRU(char addr[]){
	int highest = 0; 
	int index = get_index(addr, find_tag_length(addr)) * assoc; 
	int highest_index = index; 
	int i; 
	writes++; 
	for(i = index; i < index + assoc; i++){ 
		if(strncmp(cache[i].tag, addr, cache[i].tag_length) == 0){ 
			if(cache[i].valid == 1){
				if(cache[i].recent == 0){
					num_of_hits++; 
					strcpy(cache[i].tag, addr);
					cache[i].tag_length = find_tag_length(addr);
					update_recents(i, cache[i].set);
					return;
				}
			} 
		} 
	} 
	num_of_misses++; 
	reads++; 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].valid == 0){
			cache[i].valid = 1;
			if(cache[i].recent == 0){
				num_of_hits++; 
				strcpy(cache[i].tag, addr);
				cache[i].tag_length = find_tag_length(addr);
				update_recents(i, cache[i].set);
				return;
			} 
		} 
	} 
	index = get_index(addr, find_tag_length(addr)) * assoc; 
	i = index; 
	for(; i < index + assoc; i++){ 
		if(cache[i].recent > highest){ 
			highest = cache[i].recent; 
			highest_index = i; 
		} 
	} 
	cache[highest_index].tag_length = find_tag_length(addr); 
	strcpy(cache[highest_index].tag, addr); 
	update_recents(highest_index, cache[highest_index].set); 
}

void print_cache(){
	int i; 
	int j; 
	for(i = 0; i < num_of_lines; i++){ 
		printf("index: %d set: %d valid: %d recent: %d tag length: %d tag:", i, cache[i].set, cache[i].valid, cache[i].recent, cache[i].tag_length); 
		for(j = 0; j < cache[i].tag_length; j++){ 
			printf("%c", cache[i].tag[j]); 
		} 
		printf("\n"); 
	}
}

void free_cache(){
	int i; 
	for(i = 0; i < num_of_lines; i++){ 
		free(cache[i].tag); 
	} 
	free(cache);
}
