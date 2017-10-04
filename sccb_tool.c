#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define OV490_BANK_HIGH			0xfffd
#define OV490_BANK_LOW			0xfffe
#define DEBUG_ON 1

enum xfer_state{
	READ = 0, 
	WRITE = 1,
	NOTSET = -1};

enum reg_size{
	BYTE_REG = 0, 
	SHORT_REG = 1,
	WORD_REG = 2,
	NOTSET_REG = -1};

int file = -1;
char filename[20] = {0};

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
    printf("\nsccb -w:r <adapter> <dev_addr> <reg_size> <address> <val>\n\n");



}

/* read a register with 16bit address */
static int i2c_sccb_reg_read(uint16_t reg, uint8_t *val)
{
	int ret;
	int buf = 0;
	uint8_t data[2] = { reg >> 8, reg & 0xff };
	
	if (write(file, data, 2) != 2){
		printf("Failed writing register 0x%04x!\n", reg);
		return -1;
	}
 
	if (read(file, &buf, 1) != 1)
		goto err;
 
	*val = (uint8_t)buf;
	return 0;
err:
	printf("Failed reading register 0x%04x!\n", reg);
	return ret;
}
 
/* write data byte to a register with 16bit address */
static int i2c_sccb_reg_write(uint16_t reg, uint8_t val)
{
	int ret;
	uint8_t data[3] = { reg >> 8, reg & 0xff, val };
 
	if (write(file, data, 3) != 3){
		printf("Failed writing register 0x%04x!\n", reg);
		return -1;
	}
 
	return 0;
}
 
/* write and read 32 bit registers */
 
static int i2c_sccb_reg_read32(uint32_t reg, uint8_t *val)
{
	uint8_t bank_high = (reg >> 24) & 0xff;
	uint8_t bank_low  = (reg >> 16) & 0xff;
	uint16_t reg_addr = reg & 0xffff;
	int ret = 0;

	/* For writing a register with 32 bit address, First set the bank
	 * address by writing to two BANK address registers. Then access
	 * the register using 16LSB bits.
	 */
	ret = i2c_sccb_reg_write(OV490_BANK_HIGH, bank_high);
	if (!ret)
		ret = i2c_sccb_reg_write(OV490_BANK_LOW, bank_low);
	if (!ret)
		ret = i2c_sccb_reg_read(reg_addr, val);

	 
	#if DEBUG_ON
		printf("\n\n==> DEBUG  <==\n");
		printf("Function: i2c_sccb_reg_read32\n");
		printf("Params: reg=0x%08X\n", reg);
		printf("\nBank High: 0x%02X\n", bank_high);
		printf("Bank Low: 0x%02X\n", bank_low);
		printf("Reg Addr: 0x%04X\n", reg_addr);
		
		printf("\nReturn Value: 0x%02X\n", *val);
	#endif

	return ret;
}
 
static int i2c_sccb_reg_write32(uint32_t reg, uint8_t val)
{
	uint8_t bank_high = (reg >> 24) & 0xff;
	uint8_t bank_low  = (reg >> 16) & 0xff;
	uint16_t reg_addr = reg & 0xffff;
	int ret = 0;
 
	/* For writing a register with 32 bit address, First set the bank
	 * address by writing to two BANK address registers. Then access
	 * the register using 16LSB bits.
	 */
	ret = i2c_sccb_reg_write(OV490_BANK_HIGH, bank_high);
	if (!ret)
		ret = i2c_sccb_reg_write(OV490_BANK_LOW, bank_low);
	if (!ret)
		ret = i2c_sccb_reg_write(reg_addr, val);

	#if DEBUG_ON
		printf("\n\n==> DEBUG  <==\n");
		printf("Function: i2c_sccb_reg_read32\n");
		printf("Params: reg=0x%08X, val=0x%02X\n", reg, val);
		printf("\nBank High: 0x%02X\n", bank_high);
		printf("Bank Low: 0x%02X\n", bank_low);
		printf("Reg Addr: 0x%04X\n", reg_addr);
	#endif

	return ret;
}

static int write_sccb_register(uint32_t register_address, uint8_t register_value, enum reg_size data_width){
	int ret = 0;
	printf("Write Register: ");
	switch(data_width){
		case WORD_REG:	
			printf("0x%08X ", register_address);		
			ret = i2c_sccb_reg_write32(register_address, register_value);
		break;
		case SHORT_REG:
			printf("0x%04X ", register_address);
			ret = i2c_sccb_reg_write(register_address, register_value);
		break;
		case BYTE_REG:
		default:
			printf("\n Byte or Unknown register size not supported \n");
			ret = 1;
		break;
	}
	
	if(!ret){
		printf(" Value 0x%02X\n", register_value);
	}

	return ret;
}


static int read_sccb_register(uint32_t register_address, uint8_t* register_value, enum reg_size data_width){
	int ret = 0;
	uint8_t val;
	printf("Read Register: ");
	switch(data_width){
		case WORD_REG:		
			printf("0x%08X ", register_address);		
			i2c_sccb_reg_read32(register_address, &val);
		break;
		case SHORT_REG:
			printf("0x%04X ", register_address);	
			i2c_sccb_reg_read(register_address,  &val);
		break;
		case BYTE_REG:
		default:
			printf("\n Byte or Unknown register size not supported \n");
			ret = 1;
		break;
	}

	if(!ret){
		*register_value = val;
		printf(" Value: 0x%02X\n", *register_value);
	}

	return ret;
}


int main(int argc, char *argv[]){
	int arg = 0;
    int opt_index = 0;
    int ret = 0;
    int arg_count = 0;
    int index = 0;
    char * next;

	enum xfer_state readwrite = NOTSET;
	enum reg_size datawidth = NOTSET_REG;
	uint8_t adapter_nr = 2; 
	uint8_t dev_addr = 0x00;
	uint32_t reg_addr = 0x00;
	uint8_t reg_val = 0x00;
	
	int parsed = -1;

#if DEBUG_ON
    printf("argc: %d\n", argc);
#endif

    while (arg != -1) {
	    arg = getopt(argc, argv, "w:r:h");
        switch (arg) {
        	case 'w':
        	{     	
#if DEBUG_ON
				printf("Write Sensor\n"); 
#endif
        		readwrite = WRITE;
	            index = optind-1;
	
        		int n = 5;		
        		while(n>0){
#if DEBUG_ON
					printf("index: %d\n", index);
#endif
	            	if(index>=argc){
	            		printf("Error not enough args\n");
        				goto error_handler;
	            	}

        			next = strdup(argv[index]);
        			switch(n){
        				case 5:
							adapter_nr = atoi(next);
#if DEBUG_ON
							printf("Adapter: %d\n", adapter_nr);
#endif
        				break;
        				case 4:
							dev_addr = (int)strtol(next, NULL, 0);
#if DEBUG_ON
							printf("Device Address: 0x%02X\n", dev_addr);
#endif
        				break;
        				case 3:
        					switch((char)*next){
        						case 'w':
								case 'W':
#if DEBUG_ON
									printf("32-bit size selected\n");
#endif
									datawidth = WORD_REG;
        						break;
        						case 's':
								case 'S':
#if DEBUG_ON
									printf("16-bit size selected\n");
#endif
									datawidth = SHORT_REG;
        						break;
        						case 'b':
								case 'B':
#if DEBUG_ON
									printf("8-bit size selected\n");
#endif
									datawidth = BYTE_REG;
        						break;        						
        						default:
									printf("Invalid size opiton\n");
									datawidth = NOTSET_REG;
        						break;
        					}
#if DEBUG_ON
	            			printf("Reg_Size: %s\n", next);
#endif
        				break;
        				case 2:
							reg_addr = (uint32_t)strtoll(next, NULL, 0);
#if DEBUG_ON
							printf("Address: 0x%08X\n", reg_addr);
#endif
        				break;
        				case 1:
							reg_val = (uint8_t)strtol(next, NULL, 0);
#if DEBUG_ON
							printf("Val: 0x%02X\n", reg_val);
#endif							
        				break;
        				default:
        					printf("default handler error!!!!\n");
        					goto error_handler;
        				break;
        			}
            		
            		n--;
            		index++;
            		
				}
        	}
        	break;
        	case 'r':
        	{
#if DEBUG_ON				
				printf("Read Sensor\n"); 
#endif				
        		readwrite = READ;
	            index = optind-1;
	
        		int n = 4;		
        		while(n>0){
#if DEBUG_ON
					printf("index: %d\n", index);
#endif
	            	if(index>=argc){
	            		printf("Error not enough args\n");
        				goto error_handler;
	            	}

        			next = strdup(argv[index]);
        			switch(n){
        				case 4:
							adapter_nr = atoi(next);
#if DEBUG_ON						
							printf("Adapter: %d\n", adapter_nr);
#endif							
        				break;
        				case 3:
							dev_addr = (int)strtol(next, NULL, 0);
#if DEBUG_ON							
							printf("Device Address: 0x%02X\n", dev_addr);
#endif							
        				break;
        				case 2:
        					switch((char)*next){
        						case 'w':
								case 'W':
#if DEBUG_ON								
									printf("32-bit size selected\n");
#endif									
									datawidth = WORD_REG;
        						break;
        						case 's':
								case 'S':
#if DEBUG_ON								
									printf("16-bit size selected\n");
#endif									
									datawidth = SHORT_REG;
        						break;
        						case 'b':
								case 'B':
#if DEBUG_ON								
									printf("8-bit size selected\n");
#endif									
									datawidth = BYTE_REG;
        						break;        						
        						default:
									printf("Invalid size opiton\n");
									datawidth = NOTSET_REG;
        						break;
        					}
#if DEBUG_ON	            			
							printf("Reg_Size: %s\n", next);
#endif						

        				break;
        				case 1:
							reg_addr = (uint32_t)strtoll(next, NULL, 0);
#if DEBUG_ON							
							printf("Address: 0x%08X\n", reg_addr);
#endif							
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
		/* open file Handle */
		sprintf( filename, "/dev/i2c-%d", adapter_nr );
		if ((file = open(filename, O_RDWR)) < 0) {
		    /* ERROR HANDLING: you can check errno to see what went wrong */
		    perror("Failed to open the i2c bus");
		    exit(1);
		}

		/* open device to communicate with */
		if (ioctl(file, I2C_SLAVE_FORCE, dev_addr) < 0) {
			/* ERROR HANDLING; you can check errno to see what went wrong */
			printf("Error setting slave device\n");
			close(file);
			exit(1);
		}

		if(readwrite == WRITE){
			if(write_sccb_register(reg_addr, reg_val, datawidth)){
				printf("\nFailed to write register\n");

			}			
		}else{
			reg_val = 0;
			if(read_sccb_register(reg_addr, &reg_val, datawidth)){
				printf("\nFailed to read register\n");
			}
		}

		/* close file handle */
		if(file > 0){
			close(file);
		}

		return 0;
	}else{
		printf("Failed to parse parameters\n");
	}

error_handler:        		
    display_menu();
	return ret;
}
