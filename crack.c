#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "md5.h"

const int PASS_LEN=50;        // Maximum any password can be
const int HASH_LEN=33;        // Length of MD5 hash strings

// Stucture to hold both a plaintext password and a hash.
struct entry 
{
    char password[PASS_LEN];
    char hash[HASH_LEN];
};

//returns an int that contains the length of all the characters in the file.
int file_length(char *filename)
{
    struct stat info;
    if (stat(filename, &info) < 0 ) 
        return -1;
    else
        return info.st_size;
}

//Comparison function for qsort
int entrycomp(const void *a, const void *b)
{
    struct entry *enta = (struct entry *)a;
    struct entry *entb = (struct entry *)b;
    return strcmp(enta->hash, (*entb).hash);
}


//Comparison function for bsearch.
int bentrycomp(const void *target, const void *elem)
{
    char *target_str = (char *)target;
    struct entry *entryElem = (struct entry *)elem;
    return strcmp(target_str, (*entryElem).hash);
}

// Read in the dictionary file and return an array of structs.
// Each entry should contain both the hash and the dictionary
// word.
struct entry *read_dictionary(char *filename, int *size)
{
    int filelength = file_length(filename);
    FILE *c = fopen(filename, "r");
    if (!c)
    {
        printf("Can't open %s for reading\n", filename);
        exit(1);
    }
    
    char *contents = malloc(filelength);
    fread(contents, 1, filelength, c);
    fclose(c);
    
    // Loop through contents, replace \n with \0
    int lines = 0;
    for (int i = 0; i < filelength; i++)
    {
        if (contents[i] == '\n') {
            contents[i] = '\0';
            lines++;
        }
    }
    
    printf("lines %d\n", lines);
    
    // Allocate array of structs
    struct entry *ent = malloc(lines * sizeof(struct entry));
    
    // Copy the first entry into the entries array
    strcpy(ent[0].password, &contents[0]);
    char *hashedPass = md5(ent[0].password,strlen(ent[0].password));
    strcpy(ent[0].hash, hashedPass);
   
    int count = 1;
    for (int i = 0; i < filelength; i++)
    {
        if (contents[i] == '\0')
        {
            char *nexte = &contents[i] + 1;
            strcpy(ent[count].password, nexte);
            char *hPass = md5(ent[count].password,strlen(ent[count].password));
            strcpy(ent[count].hash, hPass);
            count++;
        }
    }
    
    *size = lines;
    return ent;
}





int main(int argc, char *argv[])
{
    if (argc < 3) 
    {
        printf("Usage: %s hash_file dict_file\n", argv[0]);
        exit(1);
    }
    
    char *hashFile = argv[1];
    char *dictFile = argv[2];
    int dlen;
    
    
    //Read the dictionary file into an array of entry structures
    struct entry *dict = read_dictionary(dictFile, &dlen);
    
    //Sort the hashed dictionary using qsort.
    printf("organizing by hash!\n");
    qsort(dict, dlen, sizeof(struct entry), entrycomp);
    
    
    // Open the hash file for reading.
    FILE* file = fopen(hashFile, "r"); /* should check the result */
    if(!file){
        
        printf("Cannot find or open file %s \n", hashFile);
        exit(1);
    }
    
    
    // For each hash, search for it in the dictionary using
    // binary search.
    // If you find it, get the corresponding plaintext dictionary word.
    // Print out both the hash and word.
    // Need only one loop. (Yay!)
    
    int findCount = 0;
    char line[HASH_LEN];
    while (fgets(line, sizeof(line), file)) {
        struct entry *found = bsearch(line, dict, dlen, sizeof(struct entry), bentrycomp);
        
        if(found)
        {
            printf("Found a match: Password: %s, Hash: %s\n", found->password,  found->hash);   
            findCount++;
        }
        
    }
    printf("Found %i matches\n", findCount);

    fclose(file);
    
}
