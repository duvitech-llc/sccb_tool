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
    printf("\nsccb -w:r <adapter> <dev_addr> <reg_size> <reg> <val>\n\n");



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
	return ret;
}

static int write_sccb_register(int register_address, int register_value, enum reg_size data_width){
	int ret = 0;


	return ret;
}


static int read_sccb_register(int register_address, int* register_value, enum reg_size data_width){
	int ret = 0;


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
	int adapter_nr = 2; 
	int dev_addr = 0x00;
	int reg_addr = 0x00;
	int reg_val = 0x00;
	
	int parsed = -1;

    printf("argc: %d\n", argc);

    while (arg != -1) {
	    arg = getopt(argc, argv, "w:r:h");
        switch (arg) {
        	case 'w':
        	{     	
        		printf("Write Sensor\n"); 
        		readwrite = WRITE;
	            index = optind-1;
	
        		int n = 5;		
        		while(n>0){

	            	printf("index: %d\n", index);
	            	if(index>=argc){
	            		printf("Error not enough args\n");
        				goto error_handler;
	            	}

        			next = strdup(argv[index]);
        			switch(n){
        				case 5:
        					adapter_nr = atoi(next);
	            			printf("Adapter: %d\n", adapter_nr);
        				break;
        				case 4:
        					dev_addr = (int)strtol(next, NULL, 0);
	            			printf("Device Address: 0x%02X\n", dev_addr);
        				break;
        				case 3:
        					switch((char)*next){
        						case 'w':
        						case 'W':
									printf("32-bit size selected\n");
									datawidth = WORD_REG;
        						break;
        						case 's':
        						case 'S':
									printf("16-bit size selected\n");
									datawidth = SHORT_REG;
        						break;
        						case 'b':
        						case 'B':
									printf("8-bit size selected\n");
									datawidth = BYTE_REG;
        						break;        						
        						default:
									printf("Invalid size opiton\n");
									datawidth = NOTSET_REG;
        						break;
        					}
	            			
	            			printf("Reg_Size: %s\n", next);

        				break;
        				case 2:
        					reg_addr = (int)strtol(next, NULL, 0);
	            			printf("Reg: 0x%08X\n", reg_addr);
        				break;
        				case 1:
	            			reg_val = (int)strtol(next, NULL, 0);
	            			printf("Val: 0x%02X\n", reg_val);
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
        		printf("Read Sensor\n"); 
        		readwrite = READ;
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
        					dev_addr = (int)strtol(next, NULL, 0);
	            			printf("Device Address: 0x%02X\n", dev_addr);
        				break;
        				case 2:
        					switch((char)*next){
        						case 'w':
        						case 'W':
									printf("32-bit size selected\n");
									datawidth = WORD_REG;
        						break;
        						case 's':
        						case 'S':
									printf("16-bit size selected\n");
									datawidth = SHORT_REG;
        						break;
        						case 'b':
        						case 'B':
									printf("8-bit size selected\n");
									datawidth = BYTE_REG;
        						break;        						
        						default:
									printf("Invalid size opiton\n");
									datawidth = NOTSET_REG;
        						break;
        					}
	            			
	            			printf("Reg_Size: %s\n", next);

        				break;
        				case 1:
        					reg_addr = (int)strtol(next, NULL, 0);
	            			printf("Reg: 0x%08X\n", reg_addr);
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
		if (ioctl(file, I2C_SLAVE, dev_addr) < 0) {
			/* ERROR HANDLING; you can check errno to see what went wrong */
			printf("Error setting slave device\n");
			close(file);
			exit(1);
		}


		printf("Run function ");
		if(readwrite == WRITE){
			printf("write \n");
			if(write_sccb_register(reg_addr, reg_val, datawidth) == -1){
				printf("Failed to write register\n");

			}
		}else{
			printf("read \n");
			reg_val = 0;
			if(read_sccb_register(reg_addr, &reg_val, datawidth) == -1){
				printf("Failed to read register\n");
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
