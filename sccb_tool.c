#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>


int file = -1;
int adapter_nr = 2; /* probably dynamically determined */
char filename[20];
int parsed = -1;

static struct option long_options[] = {
    {"write", required_argument, 0,'w'},
    {"read", required_argument, 0, 'r'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void display_menu(void){

    printf("\nSCCB Tool\n");
    printf("\nAdapter: 0-9\n");
    printf("\nRegister Size: w, s, b\n");    
    printf("\nsccb -w:r <adapter> <reg_sz> <reg> <val>\n\n");



}

static void GetArgsList (int argc, char *argv[], int* o_argc, char*** o_argv)
{
    char** ret = NULL;
    int i;
    int count = 0;

    for (i = optind - 1; i < argc ; ++i)
    {
        if (argv[i][0]=='-')
        {
            break;
        }
        else
        {
            if (NULL == ret)
            {
                ret = &argv[i];
            }
            count++;
        }
    }

    optind = i - 1;

    if (o_argc)
    {
        *o_argc = count;
    }

    *o_argv = ret;
}

int main(int argc, char *argv[]){
	int arg = 0;
    int opt_index = 0;
    int ret = 0;
    int arg_count = 0;
    int index = 0;
    char * next;

    printf("argc: %d\n", argc);

    while (arg != -1) {
	    arg = getopt(argc, argv, "w:r:h");
        switch (arg) {
        	case 'w':
        	{     	
        		printf("Write Sensor\n"); 
	            index = optind-1;
	
        		int n = 4;		
        		while(n>0){

	            	printf("index: %d\n", index);
	            	if(index>=argc){
	            		printf("Error not enough args\n");
        				goto error_handler;
	            	}

        			next = strdup(argv[index]);
        			switch(n){
        				case 4:
        					adapter_nr = atoi(next);
	            			printf("Adapter: %d\n", adapter_nr);
        				break;
        				case 3:
        					switch((char)*next){
        						case 'w':
        						case 'W':
									printf("32-bit size selected\n");
        						break;
        						case 's':
        						case 'S':
									printf("16-bit size selected\n");
        						break;
        						case 'b':
        						case 'B':
									printf("8-bit size selected\n");
        						break;        						
        						default:
									printf("Invalid size opiton\n");
        						break;
        					}
	            			
	            			printf("Reg_Size: %s\n", next);

        				break;
        				case 2:
	            			printf("Reg: 0x%08X\n", (int)strtol(next, NULL, 0));
        				break;
        				case 1:
	            			printf("Val: 0x%02X\n", (int)strtol(next, NULL, 0));
        				break;
        				default:
        					printf("default handler error!!!!\n");
        					goto error_handler;
        				break;
        			}


					//int number = (int)strtol(optarg, NULL, 0);
					//printf("set adapter to 0x%04X\n", number);

	            	//printf("optargs: %s\n", optarg);
					//l=(int)atoi(optarg);
					//adapter_nr = (int) l;
					//printf("set adapter to %d\n", adapter_nr);

            		
            		n--;
            		index++;
            		
				}
        	}
        	break;
        	case 'r':
        	{
        		printf("Read Sensor\n"); 
	            index = optind-1;
	
        		int n = 3;		
        		while(n>0){

	            	printf("index: %d\n", index);
	            	if(index>=argc){
	            		printf("Error not enough args\n");
        				goto error_handler;
	            	}

        			next = strdup(argv[index]);
        			switch(n){
        				case 3:
        					adapter_nr = atoi(next);
	            			printf("Adapter: %d\n", adapter_nr);
        				break;
        				case 2:
        					switch((char)*next){
        						case 'w':
        						case 'W':
									printf("32-bit size selected\n");
        						break;
        						case 's':
        						case 'S':
									printf("16-bit size selected\n");
        						break;
        						case 'b':
        						case 'B':
									printf("8-bit size selected\n");
        						break;        						
        						default:
									printf("Invalid size opiton\n");
        						break;
        					}
	            			
	            			printf("Reg_Size: %s\n", next);

        				break;
        				case 1:
	            			printf("Reg: 0x%08X\n", (int)strtol(next, NULL, 0));
        				break;
        				default:
        					printf("default handler error!!!!\n");
        				break;
        			}

            		n--;
            		index++;
            		
				}
        	}
        	break;
            case -1:
            	if(arg_count == 0)
    				goto error_handler;
				parsed = 1;
            break;
            case 0:
        	case 'h':
        	default:
        		goto error_handler;
        	break;
        }
		arg_count++;
	}

	if(parsed){

		printf("Run function\n");
		return 0;
	}
	
error_handler:
        		
    display_menu();

	return ret;
}