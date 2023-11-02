#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

typedef int bool;
#define true 1
#define false 0

#define DRAW_NONE			0
#define DRAW_MESH			1
#define DRAW_3D_FULLBARS	2
#define DRAW_3D_CUBES		3

#define FILETYPE_NONE		0
#define FILETYPE_CSV		1
#define FILETYPE_PGM		2


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int replacechar(char *str, char orig, char rep) {
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;
}

char* getFileExtension(char path[]) {
	char *result;
    int i, n;

    assert(path != NULL);
    n = strlen(path);
    i = n - 1;
    while ((i > 0) && (path[i] != '.') && (path[i] != '/') && (path[i] != '\\')) {
        i--;
    }
    if ((i > 0) && (i < n - 1) && (path[i] == '.') && (path[i - 1] != '/') && (path[i - 1] != '\\')) {
        result = path + i;
    } else {
        result = path + n;
    }
	return (result);
}

char* toLower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}
char* toUpper(char* s) {
  for(char *p=s; *p; p++) *p=toupper(*p);
  return s;
}

void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

int main(int argc, char** argv) {

	int fileInType = FILETYPE_NONE;
	char fileNameIn[255];
	char *fileNameInExtension;
	char fileNameOutMid[255];
	char *fileNameOut;
	char objExtension[] = ".obj";
	int bIsFileInGiven = false;

	char** tokens;
	
	int countRows = 0;
	int countCols = 0;
	int maxRows = 0;
	int maxCols = 0;

	int numArguments = (argc-1);
	if (numArguments>0) {
		if (numArguments>0) {
			strcpy(fileNameIn, argv[1]);
			bIsFileInGiven = true;
		}
	}
	if (bIsFileInGiven==false) {
		printf("Arguments missing:\n");
		printf("\t%s <filenameIn>\n", argv[0]);
		return(1);
	}

	fileNameInExtension = getFileExtension(fileNameIn);
	printf("IN.extension ==> %s\n", toLower(fileNameInExtension));
	strcpy(fileNameOutMid, fileNameIn);
	fileNameOut = strcat(fileNameOutMid, objExtension);
	printf("OUT ==> %s\n", fileNameOut);
	if (strcmp(toLower(fileNameInExtension), ".csv") == 0) {
		fileInType = FILETYPE_CSV;
		printf("File IN: .CSV...\n");
	}
	if (strcmp(toLower(fileNameInExtension), ".pgm") == 0) {
		fileInType = FILETYPE_PGM;
		printf("File IN: .PGM...\n");
	}	
	
	
    FILE * fp;
	FILE * pFileOut;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;


	// Determina numero ROWS e COLS dei dati
    fp = fopen(fileNameIn, "r");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
	}
	if (fileInType == FILETYPE_CSV) {
		while ((read = getline(&line, &len, fp)) != -1) {
			replacechar(line, '\n', ' ');
			//printf("Retrieved line of length %zu:\n", read);
			tokens = str_split(line, '\t');
			if (tokens) {
				int i;
				countCols=0;
				for (i = 0; *(tokens + i); i++) {
					replacechar(*(tokens + i), ',', '.');
					trim(*(tokens + i));
					//printf("[%s]  ", *(tokens + i));
					free(*(tokens + i));
					countCols++;
				}
				//printf("\n");
				free(tokens);
			}
			//printf("\n");
			countRows++;
		}	
	} else if (fileInType == FILETYPE_PGM) {
		int countLinesRead = 0;
		while ((read = getline(&line, &len, fp)) != -1) {
			if (line[0]!='#') {
				countLinesRead++;
				if (countLinesRead==2) {
					break;
				}
			}
		}
		tokens = str_split(line, ' ');
		// printf("%s / %s\n", tokens[0], tokens[1]);
		if (tokens) {
			countCols = atoi(tokens[0]);
			countRows = atoi(tokens[1]);
		}

	}
    fclose(fp);
	
	maxRows = countRows;
	maxCols = countCols;
	printf("numRows=%i / numCols=%i\n", maxRows, maxCols);
	

	// Alloca memoria sufficiente per i dati
	float* ptrArray = malloc((countRows * countCols+1) * sizeof(float));
    for (int i = 0; i < maxRows; i++) {
        for (int j = 0; j < maxCols; j++) {
			ptrArray[i * maxCols + j] = 0;	// init
		}
    }
	
	// Legge i dati
	fp = fopen(fileNameIn, "r");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
	}
	countRows = 0;
	countCols = 0;
	if (fileInType == FILETYPE_CSV) {
		while ((read = getline(&line, &len, fp)) != -1) {
			replacechar(line, '\n', ' ');
			tokens = str_split(line, '\t');
			if (tokens) {
				int i;
				countCols=0;
				for (i = 0; *(tokens + i); i++) {
					replacechar(*(tokens + i), ',', '.');
					trim(*(tokens + i));
					ptrArray[countRows*maxCols + countCols] = atof(*(tokens + i));
					free(*(tokens + i));
					countCols++;
				}
				free(tokens);
			}
			countRows++;
		}
	} else if (fileInType == FILETYPE_PGM) {
		int countLinesRead = 0;
		while ((read = getline(&line, &len, fp)) != -1) {
			if (line[0]!='#') {
				countLinesRead++;
				if (countLinesRead>3) {
					break;
				}
			}
		}
		replacechar(line, '\n', ' ');
		int strlen_line = maxCols*maxRows;
		tokens = str_split(line, ' ');
		//printf("===%i\n", strlen_line);
		if (tokens) {
			for (int i = 0; i<strlen_line; i++) {
				//printf("[%i]==", i);
				if(tokens[i] != NULL) {
					//printf("==%s\n", tokens[i]);
					replacechar(tokens[i], ',', '.');
					trim(tokens[i]);
					ptrArray[i] = atof(tokens[i]);			
					free(tokens[i]);
				}

			}
			free(tokens);
		}
	}
    fclose(fp);

	printf("Now writing OBJ file...\n");

	pFileOut = fopen(fileNameOut,"w");
	if (pFileOut==NULL) {
		printf("ERROR: Error opening file\n");
		exit(1);
	}
	
	int whatToDraw = DRAW_NONE;
	whatToDraw = DRAW_MESH;
	//whatToDraw = DRAW_3D_FULLBARS;
	// whatToDraw = DRAW_3D_CUBES;
	
	if (whatToDraw==DRAW_MESH) {
		// Write all vertexes
		for (int i = 0; i < maxRows; i++) {
			for (int j = 0; j < maxCols; j++) {
				fprintf(pFileOut, "v\t%f\t%f\t%f\n", (float) i, ptrArray[i * maxCols + j], (float) -j);
			}
		}
		int p1, p2, p3, p4;
		// 1  2
		// 4 (3)
		for (int r = 1; r < maxRows; r++) {
			for (int c = 1; c < maxCols; c++) {
				p1 = (r-1) * maxCols + (c-1) +1;
				p2 = (r-1) * maxCols + (c) +1;
				p3 = r * maxCols + c +1;
				p4 = r * maxCols + (c-1) +1;
				fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p1, p2, p3, p4);
			}
			
		}
		
    } else if (whatToDraw==DRAW_3D_FULLBARS) {
		// Write all vertexes
		float v1x, v2x, v3x, v4x, v5x, v6x, v7x, v8x;
		float v1y, v2y, v3y, v4y, v5y, v6y, v7y, v8y;
		float v1z, v2z, v3z, v4z, v5z, v6z, v7z, v8z;
		float hTop, hBottom;	
		for (int r = 0; r < maxRows; r++) {
			for (int c = 0; c < maxCols; c++) {
				hTop=ptrArray[r * maxCols + c];
				if (hTop>0) {
					hBottom=0;				
					v1x=(r-0.5);	v1y=hTop;	v1z=(c-0.5);
					v2x=(r-0.5);	v2y=hTop;	v2z=(c+0.5);
					v3x=(r+0.5);	v3y=hTop;	v3z=(c+0.5);
					v4x=(r+0.5);	v4y=hTop;	v4z=(c-0.5);
					v5x=(r-0.5);	v5y=hBottom;	v5z=(c-0.5);
					v6x=(r-0.5);	v6y=hBottom;	v6z=(c+0.5);
					v7x=(r+0.5);	v7y=hBottom;	v7z=(c+0.5);
					v8x=(r+0.5);	v8y=hBottom;	v8z=(c-0.5);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v1x, v1y, v1z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v2x, v2y, v2z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v3x, v3y, v3z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v4x, v4y, v4z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v5x, v5y, v5z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v6x, v6y, v6z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v7x, v7y, v7z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v8x, v8y, v8z);
				}
			}
		}
		int pBase, p1, p2, p3, p4, p5, p6, p7, p8;
		// 1  2
		// 4 (3)
		pBase = 0;
		for (int r = 0; r < maxRows; r++) {
			for (int c = 0; c < maxCols; c++) {
				hTop=ptrArray[r * maxCols + c];
				if (hTop>0) {				
					p1 = pBase + 1;
					p2 = pBase + 2;
					p3 = pBase + 3;
					p4 = pBase + 4;
					p5 = pBase + 5;
					p6 = pBase + 6;
					p7 = pBase + 7;
					p8 = pBase + 8;
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p1, p2, p3, p4);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p1, p4, p8, p5);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p4, p3, p7, p8);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p3, p2, p6, p7);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p2, p1, p5, p6);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p5, p8, p7, p6);
					//
					pBase += 8;
				}
			}
		}
    } else if (whatToDraw==DRAW_3D_CUBES) {
		// Write all vertexes
		float v1x, v2x, v3x, v4x, v5x, v6x, v7x, v8x;
		float v1y, v2y, v3y, v4y, v5y, v6y, v7y, v8y;
		float v1z, v2z, v3z, v4z, v5z, v6z, v7z, v8z;
		float hTop, hBottom;	
		for (int r = 0; r < maxRows; r++) {
			for (int c = 0; c < maxCols; c++) {
				hTop=ptrArray[r * maxCols + c];
				if (hTop>0) {
					hBottom=hTop-1;				
					v1x=(r-0.5);	v1y=hTop;	v1z=(c-0.5);
					v2x=(r-0.5);	v2y=hTop;	v2z=(c+0.5);
					v3x=(r+0.5);	v3y=hTop;	v3z=(c+0.5);
					v4x=(r+0.5);	v4y=hTop;	v4z=(c-0.5);
					v5x=(r-0.5);	v5y=hBottom;	v5z=(c-0.5);
					v6x=(r-0.5);	v6y=hBottom;	v6z=(c+0.5);
					v7x=(r+0.5);	v7y=hBottom;	v7z=(c+0.5);
					v8x=(r+0.5);	v8y=hBottom;	v8z=(c-0.5);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v1x, v1y, v1z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v2x, v2y, v2z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v3x, v3y, v3z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v4x, v4y, v4z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v5x, v5y, v5z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v6x, v6y, v6z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v7x, v7y, v7z);
					fprintf(pFileOut, "v\t%f\t%f\t%f\n", v8x, v8y, v8z);
				}
			}
		}
		int pBase, p1, p2, p3, p4, p5, p6, p7, p8;
		// 1  2
		// 4 (3)
		pBase = 0;
		for (int r = 0; r < maxRows; r++) {
			for (int c = 0; c < maxCols; c++) {
				hTop=ptrArray[r * maxCols + c];
				if (hTop>0) {				
					p1 = pBase + 1;
					p2 = pBase + 2;
					p3 = pBase + 3;
					p4 = pBase + 4;
					p5 = pBase + 5;
					p6 = pBase + 6;
					p7 = pBase + 7;
					p8 = pBase + 8;
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p1, p2, p3, p4);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p1, p4, p8, p5);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p4, p3, p7, p8);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p3, p2, p6, p7);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p2, p1, p5, p6);
					fprintf(pFileOut, "f\t%i\t%i\t%i\t%i\n", p5, p8, p7, p6);
					//
					pBase += 8;
				}
			}
		}
	}
	
	
    fclose(pFileOut);	
	
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
	
}
	
	