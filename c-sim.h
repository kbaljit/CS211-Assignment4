//Baljit Kaur
#ifndef C_SIM_H
#define C_SIM_H

typedef struct Line{ 
	int valid; 
	char *tag; 
	char *block; 
	int set; 
	int recent; 
	int dirty; 
	int tag_length; 
}Line; 

Line *create_cache();
int find_tag_length(char address[]);
char *get_binary_addr(char address[]);
int get_index(char address[], int tag_length);
void update_recents(int new, int index);
void write_to_cache(char address[]);
void read_from_cache(char address[]);
void write_to_cache_wb(char address[]);
void read_from_cache_wb(char address[]);
void print_cache();
void free_cache();

#endif 

