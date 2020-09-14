#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
char* shuffle_sample(char* s){
     char *return_str = (char*)malloc(10*sizeof(char));
     if (strlen(s) == 4){
         return_str[0] = s[2];
         return_str[1] = s[3];
         return_str[2] = s[0];
         return_str[3] = s[1];
     }else{
        return_str[0] = s[3];
        return_str[1] = s[4];
        return_str[2] = s[0];
         return_str[3] = s[1];
     }
     return return_str;
 }

signed short hex_to_decimal(char* str){
    char *rest;
    return  (signed) (short) strtol(str, &rest, 16);
}

float calcluate(char* pt1, char* pt2){
    char* sample_rate1 = shuffle_sample(pt1);
    char* sample_rate2 = shuffle_sample(pt2);
    sample_rate2[4] = *sample_rate1;
    sample_rate2[5] = *(sample_rate1+1);
    sample_rate2[6] = *(sample_rate1+2);
    sample_rate2[7] = *(sample_rate1+3);
    char *rest;
    float x = strtol(sample_rate2, &rest, 16);
    free(sample_rate1);
    free(sample_rate2);
    return x;
}

void process_sample2(char *pointer, double rate){ 
    char *str = shuffle_sample(pointer);
    signed short value = hex_to_decimal(str);
    free(str);
    value =value * rate;
    char hex[10];
    char* str2;
    if (value >= 0){
        sprintf(hex, "%04x",  value);
        str2 = shuffle_sample(hex);
    }else{
        sprintf(hex, "%x",  value);
        str2 = shuffle_sample(&hex[4]);
    }
     *pointer = str2[0];
    *(pointer+1) = str2[1];
    *(pointer+3) = str2[2];
    *(pointer+4) = str2[3];
     free(str2);
}

void pan(float x){
    char line[256];
    double processed_sample = 0.0;
    double total_sample = 0.0;
    int i = 1;
    double count = 0.0;
    double total_sample_inhex = 0.0;
    while( fgets(line, sizeof(line), stdin) != NULL ){
        if (i == 2){
            float sample_rate = calcluate(&line[34],&line[40]); 
            float second = x/1000.0;
            total_sample = second * sample_rate;
            char *str = shuffle_sample(&line[28]);
            if (hex_to_decimal(str) == 1){
                free(str);
                printf("Error processing WAV file.\n");
                break;
            }
            free(str);
        }
        if (i == 3){
            total_sample_inhex = calcluate(&line[34],&line[40]) /4;
        }
        if (i >= 3){
            int index = (i == 3) ? 3:0;
            for (; index != 4; index ++ ){
                if (count == total_sample_inhex){
                    break;
                }
                if (processed_sample == total_sample){
                    char *pt = &line[10 +index*12];
                     process_sample2(pt,0.0);
                }
                if (processed_sample <= total_sample-1){
                    double rate2 = processed_sample / total_sample;
                    double rate1 = (total_sample - processed_sample -1) / total_sample;
                    char *pt = &line[10 +index*12];
                     process_sample2(pt,rate1); 
                     process_sample2(pt+6,rate2); 
                     processed_sample += 1.0;
                }
                count += 1.0;
            }
        }
        printf("%s",line);
        i+=1;
    }
}

void fout(float x){
    char line[256];
    double processed_sample = 0.0;
    double sample_need_modify = 0.0;
    double total_sample = 0.0;
    int i = 1;
    short channel;
    while( fgets(line, sizeof(line), stdin) != NULL ){
        if (i == 2){
             float sample_rate = calcluate(&line[34],&line[40]); 
            float second = x/1000.0;
            sample_need_modify = second * sample_rate;
            char *str = shuffle_sample(&line[28]);
            channel = hex_to_decimal(str);
            free(str);
        }
        if(i == 3 && channel == 2){
             total_sample = calcluate(&line[34],&line[40]) /4;
        }
        if(i == 3 && channel == 1){
            total_sample = calcluate(&line[34],&line[40]) /2;
        }
        if (i >= 3){
            int index = (i == 3) ? 6:0;
             for (; index != 8; index ++ ){
                 if (processed_sample == total_sample){
                     break;
                 }
                 if (processed_sample >= total_sample - sample_need_modify){
                    double rate = (total_sample - processed_sample -1) / sample_need_modify;
                    char *pt = &line[10 +index*6];
                    if (channel == 2){
                        process_sample2(pt,rate); 
                        process_sample2(pt+6,rate); 
                    }else{
                        process_sample2(pt,rate); 
                    }
                }
                if (channel == 2){
                    index += 1;
                }
                processed_sample += 1.0;
            }
        }
        printf("%s",line);
         i += 1;
    }
}

void fin(float x){
    char line[256];
    double processed_sample = 0.0;
    double total_sample = 0.0;
    int i = 1;
    short channel;
    while( fgets(line, sizeof(line), stdin) != NULL ){
        if (i == 2){
            float sample_rate = calcluate(&line[34],&line[40]); 
            float second = x/1000.0;
            total_sample = second * sample_rate;
            char *str = shuffle_sample(&line[28]);
            channel = hex_to_decimal(str);
            free(str);
        }else if (i >= 3){
            int index = (i == 3) ? 6:0;
            for (; index != 8; index ++){
                 if (processed_sample <= total_sample-1){
                     double rate = processed_sample / total_sample;
                     char *pt = &line[10 +index*6];
                     if (channel == 2){
                         process_sample2(pt,rate);
                         process_sample2(pt+6,rate);
                         index += 1;
                     }else{
                         process_sample2(pt,rate);
                     }
                     processed_sample += 1.0;
                 }                    
             }
         }
        printf("%s",line);
        i += 1;
     }
}
int main(int argc, char** argv) {
    bool error = false;
    if (argc != 3){
        printf("Error: Invalid command-line arguments.\n");
        printf("Usage: ./effects -fin -fout -pan time_length\n");
        error = true;
    }else if (strncmp(argv[1],"-fin",4) !=0 && strncmp(argv[1],"-fout",4) !=0 && strncmp(argv[1],"-pan",4) !=0){
        printf("Error: Invalid command-line arguments.\n");
        printf("The first argument should be either -fin or -fout or -pan\n");
        error = true;}
    if (!error){
        for (int i=0;i<strlen(argv[2]); i++){
            if (!isdigit(argv[2][i])){
                printf("Error: Invalid command-line arguments.\n");
                printf("The second argument should be a number.\n");
                error = true;
                break;
            }
        }
    }
    if ( !error && strncmp(argv[1],"-fin",4) ==0){
        float x = strtol(argv[2], NULL, 10);
        fin(x);
    }else if ( !error && strncmp(argv[1],"-fout",4) ==0){
        float y = strtol(argv[2], NULL, 10);
        fout(y);
    }else if ( !error && strncmp(argv[1],"-pan",4) ==0){
        float y = strtol(argv[2], NULL, 10);
        pan(y);
    }
    
}