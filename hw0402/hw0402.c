//
//  hw0402.c
//  hw0402
//
//  Created by michaelleong on 16/05/2021.
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

struct option long_options[] = {
    {"level", 1, NULL, 'l'},
    {"input", 1, NULL, 'i'},
    {"output", 1, NULL, 'o'},
    {"help", 0, NULL, 'h'},
    { 0, 0, 0, 0},
};

typedef struct _varName {
    char oriVarName[129];
    char newVarName[129];
} varName;

typedef struct _funcName {
    char oriFuncName[129];
    char newFuncName[129];
} funcName;

uint64_t util_getFdSize(int fd);
int varOrFunc(char *i, char *name, int offset);
size_t checkRandomNamechar(char *name, varName *varNameList, size_t varNameListLen, funcName *funcNameList, size_t funcNameListLen);
size_t checkList(char *name, varName *varNameList, size_t varNameListLen, funcName *funcNameList, size_t funcNameListLen);
void remove_spaces(char* s);
int isLetter(char letter);
int isAscDigit(char digit);
int getIntLen(int num);
void addRandSpaceNL(FILE *writeFile);
void initVarList(varName *varNameList, size_t varNameListLen);
void initFuncList(funcName *funcNameList, size_t FuncNameListLen);
void updateFuncList(funcName *funcNameList, size_t funcNameListLen);
void printHelp(void);

int main(int argc, char *argv[]) {
    int sourceCodeFD = 0; //source code file descriptor
    int32_t c = 0;
    int32_t index = 0;
    char sourceCodeName[129] = {0};
    char outFileName[129] = {0};
    long int option = 0;
    char *pEnd = NULL;
    
    while ( ( c = getopt_long( argc, argv, "l:i:o:h", long_options, &index ) ) != -1 )
    {
        //printf( "index: %d\n", index );
        switch( c )
        {
            case 'l':
                //printf("option: l, %s\n", optarg);
                option = strtol(optarg, &pEnd, 10);
                if(pEnd == NULL) {
                    printf("Conversion unsuccessful\n");
                    return 1;
                }
                //printf("option: %ld\n", option);
                break;
                
            case 'i':
                //printf("option: i, %s\n", optarg);
                strncpy(sourceCodeName, optarg, strlen(optarg));
                //printf("fileName: %s\n", sourceCodeName);
                break;
                
            case 'o':
                //printf("option: o, %s\n", optarg);
                strncpy(outFileName, optarg, strlen(optarg));
                //printf("fileName: %s\n", outFileName);
                break;
            case 'h':
                printHelp();
                return 0;
            case '?':
                printf("invalid option >:[\n");
                printf("use '--help for more options'");
                break;
            default:
                printf("invalid option >:[\n");
                printf("use '--help for more options'");
                break;
        }
    }
    
    if(option > 4 || option < 1) {
        printf("invalid option >:[\n");
        printf("use '--help' for more options\n");
        return 1;
    }
    
    varName varNameList[129];
    funcName funcNameList[129];
    size_t varNameIndex = 0;
    size_t funcNameIndex = 0;
    
    srand((unsigned int)time(NULL));
    initVarList(varNameList, sizeof(varNameList)/sizeof(varNameList[0]));
    initFuncList(funcNameList, sizeof(funcNameList)/sizeof(funcNameList[0]));
    
    sourceCodeFD = open(sourceCodeName, O_RDWR);
    
    //open the file descriptor
    if(sourceCodeFD == -1 )
    {
        printf( "Open file error!\n" );
        printf("use '--help' for more options\n");
        return -1;
    }
    
    char *sourceCodePointer = NULL;
    uint64_t sourcefileSize = util_getFdSize(sourceCodeFD);
    sourceCodePointer = mmap( 0, sourcefileSize, PROT_READ | PROT_WRITE, MAP_SHARED, sourceCodeFD, 0 );
    
    for(char *i = sourceCodePointer; *i != 0; i++) {
        //ignore macro
        if(*i == '#') {
            while(*i != '\n') {
                i++;
            }
        }
        
        //ignore comments
        if(*i == '/' && *(i+1) == '/') {
            while(*i != '\n') {
                i++;
            }
        }
        
        if(*i == '/' && *(i+1) == '*') {
            while(*i == '*' && *(i+1) == '/') {
                i++;
            }
            i++; //get rid of the last '/'
        }
        
        //ignore strings
        if(*i == '"') {
            do {
                i++;
            } while(*i != '"');
        }
        
        //search for types
        if((strncmp(i, "int", strlen("int")) == 0) && (!isLetter(*(i+strlen("int"))))) {
            char nameTemp[129] = {0};
            if(varOrFunc(i, nameTemp, strlen("int ")) == 1) {
                //variable
                size_t index = checkList(nameTemp, varNameList, sizeof(varNameList)/sizeof(varNameList[0]), NULL, 0);
                if(index == -1) {
                    strncpy(varNameList[varNameIndex].oriVarName, nameTemp, strlen(nameTemp));
                    varNameIndex++;
                }
            } else if(varOrFunc(i, nameTemp, strlen("int ")) == 2) {
                //function
                size_t index = checkList(nameTemp, NULL, 0, funcNameList, sizeof(funcNameList)/sizeof(funcNameList[0]));
                if(index == -1) {
                    strncpy(funcNameList[funcNameIndex].oriFuncName, nameTemp, strlen(nameTemp));
                    funcNameIndex++;
                }
            }
        }
        
        if((strncmp(i, "char", strlen("char")) == 0) && (!isLetter(*(i+strlen("char"))))) {
            char nameTemp[129] = {0};
            if(varOrFunc(i, nameTemp, strlen("char ")) == 1) {
                //variable
                size_t index = checkList(nameTemp, varNameList, sizeof(varNameList)/sizeof(varNameList[0]), NULL, 0);
                if(index == -1) {
                    strncpy(varNameList[varNameIndex].oriVarName, nameTemp, strlen(nameTemp));
                    varNameIndex++;
                }
            } else if(varOrFunc(i, nameTemp, strlen("char ")) == 2) {
                //function
                size_t index = checkList(nameTemp, NULL, 0, funcNameList, sizeof(funcNameList)/sizeof(funcNameList[0]));
                if(index == -1) {
                    strncpy(funcNameList[funcNameIndex].oriFuncName, nameTemp, strlen(nameTemp));
                    funcNameIndex++;
                }
            }
        }
    }
    //main function name cannot be changed
    updateFuncList(funcNameList, sizeof(funcNameList)/sizeof(funcNameList[0]));
    /*
    printf("variables:\n");
    for(size_t i = 0; i < varNameIndex; i++) {
        printf("%s : %s\n", varNameList[i].oriVarName, varNameList[i].newVarName);
    }
    
    printf("\nfunctions:\n");
    for(size_t i = 0; i < funcNameIndex; i++) {
        printf("%s : %s\n", funcNameList[i].oriFuncName, funcNameList[i].newFuncName);
    }*/
    
    //open output file
    FILE *outFile = NULL;
    if((outFile = fopen(outFileName, "w")) == NULL) {
        printf("output file open error\n");
        printf("use '--help' for more options\n");
        return 1;
    }
    
    char *lastFound = sourceCodePointer;
    for(char *i = sourceCodePointer; *i != 0; i++) {
        //ignore macro
        if(*i == '#') {
            while(*i != '\n') {
                i++;
            }
        }
        
        //ignore comments
        if(*i == '/' && *(i+1) == '/') {
            while(*i != '\n') {
                i++;
            }
        }
        
        if(*i == '/' && *(i+1) == '*') {
            while(*i == '*' && *(i+1) == '/') {
                i++;
            }
            i++; //get rid of the last '/'
        }
        
        //ignore strings
        if(*i == '"') {
            do {
                i++;
            } while(*i != '"');
        }
        
        if(option >= 1) {
            //search for types
            if((strncmp(i, "int", strlen("int")) == 0) && (!isLetter(*(i+strlen("int"))))) {
                addRandSpaceNL(outFile);
                i += strlen("int");
                addRandSpaceNL(outFile);
            }
            
            if((strncmp(i, "char", strlen("char")) == 0) && (!isLetter(*(i+strlen("char"))))) {
                addRandSpaceNL(outFile);
                i += strlen("char");
                addRandSpaceNL(outFile);
            }
        }
        
        
        //search variables
        for(size_t j = 0; j < varNameIndex; j++) {
            if((strncmp(i, varNameList[j].oriVarName, strlen(varNameList[j].oriVarName)) == 0) && (!isLetter(*(i+strlen(varNameList[j].oriVarName))))) {
                //printf("offset: %ld\n", i-lastFound);
                if(option == 1) {
                    addRandSpaceNL(outFile);
                    fwrite(lastFound, i-lastFound, 1, outFile);
                    fwrite(varNameList[j].oriVarName, strlen(varNameList[j].oriVarName), 1, outFile);
                    i += strlen(varNameList[j].oriVarName);
                    lastFound = i;
                    addRandSpaceNL(outFile);
                }
                
                //replace variable
                if(option >= 2) {
                    addRandSpaceNL(outFile);
                    fwrite(lastFound, i-lastFound, 1, outFile);
                    fwrite(varNameList[j].newVarName, strlen(varNameList[j].newVarName), 1, outFile);
                    i += strlen(varNameList[j].oriVarName);
                    lastFound = i;
                    addRandSpaceNL(outFile);
                }
                
                break;
            }
        }
        
        //search functions
        for(size_t j = 0; j < funcNameIndex; j++) {
            if((strncmp(i, funcNameList[j].oriFuncName, strlen(funcNameList[j].oriFuncName)) == 0) && (!isLetter(*(i+strlen(funcNameList[j].oriFuncName))))) {
                //printf("offset: %ld\n", i-lastFound);
                if(option == 1) {
                    addRandSpaceNL(outFile);
                    fwrite(lastFound, i-lastFound, 1, outFile);
                    fwrite(funcNameList[j].oriFuncName, strlen(funcNameList[j].oriFuncName), 1, outFile);
                    i += strlen(funcNameList[j].oriFuncName);
                    lastFound = i;
                    addRandSpaceNL(outFile);
                }
                
                //replace functions
                if(option >= 3) {
                    addRandSpaceNL(outFile);
                    fwrite(lastFound, i-lastFound, 1, outFile);
                    fwrite(funcNameList[j].newFuncName, strlen(funcNameList[j].newFuncName), 1, outFile);
                    i += strlen(funcNameList[j].oriFuncName);
                    lastFound = i;
                    addRandSpaceNL(outFile);
                }
                
                break;
            }
        }
        
        if(option == 4) {
            //replace numbers
            if(isAscDigit(*i)) {
                long long int intTemp = 0;
                int decPlace = 0;
                while(isAscDigit(*i)) {
                    i++;
                    decPlace++;
                }
                char tempChar = *i;
                *i = 0;
                intTemp = strtol(i-decPlace, &pEnd, 10);
                if(pEnd == NULL) {
                    printf("Conversion unsuccessful\n");
                    return 1;
                }
                *i = tempChar;
                
                //fwrite(lastFound, i-lastFound-decPlace, 1, stdout);
                fwrite(lastFound, i-lastFound-decPlace, 1, outFile);
                lastFound = i;
                //printf("numbers: %lld : ", intTemp);
                if(intTemp > 1) {
                    long long int subtractor = (rand()%(intTemp-1))+1;
                    long long int multiplier = (rand()%(intTemp-subtractor))+(intTemp-subtractor);
                    //printf("(%lld + (%lld/%lld))", subtractor, (intTemp-subtractor)*multiplier, multiplier);
                    fprintf(outFile, "(%lld + (%lld/%lld))", subtractor, (intTemp-subtractor)*multiplier, multiplier);
                } else {
                    fprintf(outFile, "%lld", intTemp);
                }
                //printf("\n");
            }
        }
        
        
    }
    
    //write the rest
    for(char *i = lastFound; *i != 0; i++) {
        fputc(*i, outFile);
    }
    
    fclose(outFile);
    
    
    munmap(sourceCodePointer, sourcefileSize);
    close(sourceCodeFD);
    return 0;
}

void initVarList(varName *varNameList, size_t varNameListLen) {
    int randomStrLen = rand() % 100 + 5;
    
    for(size_t i = 0; i < varNameListLen; i++) {
        memset((varNameList+i)->oriVarName, 0, 129);
        memset((varNameList+i)->newVarName, 0, 129);
        char tempStr[129] = {0};
        for(size_t j = 0; j < randomStrLen; j++) {
            int r = 97+(rand()%26);
            char charTemp[2] = {0};
            charTemp[0] = r;
            strncat(tempStr, charTemp, strlen(charTemp));
        }
        size_t index = checkRandomNamechar(tempStr, varNameList, varNameListLen, NULL, 0);
        while(index != -1) {
            memset(tempStr, 0, 129);
            for(size_t j = 0; j < randomStrLen; j++) {
                int r = 97+(rand()%26);
                char charTemp[2] = {0};
                charTemp[0] = r;
                strncat(tempStr, charTemp, strlen(charTemp));
            }
            index = checkRandomNamechar(tempStr, varNameList, varNameListLen, NULL, 0);
        }
        strncpy((varNameList+i)->newVarName, tempStr, strlen(tempStr));
    }
}

void initFuncList(funcName *funcNameList, size_t funcNameListLen) {
    int randomStrLen = rand() % 100 + 5;
    
    for(size_t i = 0; i < funcNameListLen; i++) {
        memset((funcNameList+i)->oriFuncName, 0, 129);
        memset((funcNameList+i)->newFuncName, 0, 129);
        char tempStr[129] = {0};
        for(size_t j = 0; j < randomStrLen; j++) {
            int r = 97+(rand()%26);
            char charTemp[2] = {0};
            charTemp[0] = r;
            strncat(tempStr, charTemp, strlen(charTemp));
        }
        size_t index = checkRandomNamechar(tempStr, NULL, 0, funcNameList, funcNameListLen);
        while(index != -1) {
            memset(tempStr, 0, 129);
            for(size_t j = 0; j < randomStrLen; j++) {
                int r = 97+(rand()%26);
                char charTemp[2] = {0};
                charTemp[0] = r;
                strncat(tempStr, charTemp, strlen(charTemp));
            }
            index = checkRandomNamechar(tempStr, NULL, 0, funcNameList, funcNameListLen);
        }
        strncpy((funcNameList+i)->newFuncName, tempStr, strlen(tempStr));
    }
}

void updateFuncList(funcName *funcNameList, size_t funcNameListLen) {
    //main function name cannot be changed
    for(size_t i = 0; i < funcNameListLen; i++) {
        if(strncmp(funcNameList[i].oriFuncName, "main", strlen("main")) == 0) {
            memset(funcNameList[i].newFuncName, 0, 129);
            strncpy(funcNameList[i].newFuncName, "main", strlen("main"));
        }
    }
}

size_t checkRandomNamechar(char *name, varName *varNameList, size_t varNameListLen, funcName *funcNameList, size_t funcNameListLen) {
    //return index
    //else return -1
    if(varNameList != NULL) {
        for(size_t i = 0; i < varNameListLen; i++) {
            if(strncmp(name, (varNameList+i)->newVarName, strlen(name)) == 0) {
                return i;
            }
        }
    }
    
    if(funcNameList != NULL) {
        for(size_t i = 0; i < funcNameListLen; i++) {
            if(strncmp(name, (funcNameList+i)->newFuncName, strlen(name)) == 0) {
                return i;
            }
        }
    }
    
    return -1;
}

size_t checkList(char *name, varName *varNameList, size_t varNameListLen, funcName *funcNameList, size_t funcNameListLen) {
    //return index
    //else return -1
    if(varNameList != NULL) {
        for(size_t i = 0; i < varNameListLen; i++) {
            if(strncmp(name, (varNameList+i)->oriVarName, strlen(name)) == 0 && (!isLetter(*((varNameList+i)->oriVarName+strlen(name))))) {
                return i;
            }
        }
    }
    
    if(funcNameList != NULL) {
        for(size_t i = 0; i < funcNameListLen; i++) {
            if(strncmp(name, (funcNameList+i)->oriFuncName, strlen(name)) == 0 && (!isLetter(*((funcNameList+i)->oriFuncName+strlen(name))))) {
                return i;
            }
        }
    }
    
    return -1;
}

void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

int isLetter(char letter) {
    if(letter >= 65 && letter <= 90) {
        return 1;
    } else if(letter >= 97 && letter <= 122) {
        return 1;
    }
    
    return 0;
}

int isAscDigit(char digit) {
    return (digit >= '0' && digit <= '9');
}

int getIntLen(int num) {
    int count = 0;
    
    while(num > 0) {
        num /= 10;
        count++;
    }
    
    return count;
}

void addRandSpaceNL(FILE *writeFile) {
    
    int spaceRep = (rand()%20)+10;
    int nlRep = (rand()%5)+10;
    
    for(size_t i = 0; i < nlRep; i++) {
        fputc('\n', writeFile);
    }
    
    for(size_t i = 0; i < spaceRep; i++) {
        fputc(' ', writeFile);
    }
}

int varOrFunc(char *i, char *name, int offset) {
    //return 1 if variable
    //return 2 if function
    //else return 0
    
    for(char *j = i; 1; j++) {
        if(*j == '(') {
            char temp = *j;
            *j = 0;
            memset(name, 0, strlen(name));
            strncpy(name, i+offset, strlen(i+offset));
            *j = temp;
            i = ++j;
            remove_spaces(name);
            return 2;
        } else if(*j == '=' || *j == ';' || *j == ')' || *j == '[') {
            char temp = *j;
            *j = 0;
            memset(name, 0, strlen(name));
            strncpy(name, i+offset, strlen(i+offset));
            *j = temp;
            i = ++j;
            remove_spaces(name);
            return 1;
        }
    }
    
    return 0;
}

uint64_t util_getFdSize( int fd )
{
    struct stat statbuf;
    
    if( fstat( fd, &statbuf ) < 0 )
    {
        close( fd );
        return -1;
    }
    
    return statbuf.st_size;
}

void printHelp() {
    printf("____usage____\n");
    printf("use './hw0402 -l <1-4> -i test.c -o test_output.c'\n");
    printf("-l is level\n");
    printf("level 1: add random spaces and new lines\n");
    printf("level 2: replace variable names with random string\n");
    printf("level 3: replace declared function names with random string\n");
    printf("*note ANSI-C function and main functions will not be replaced\n");
    printf("level 4: integers will be replace with a random formula that returns the same value\n");
}
