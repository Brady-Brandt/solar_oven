#include "hardware/flash.h"
#include "hardware/sync.h"
#include "state.h"
#include <stdint.h>

#define FLASH_LOCATION (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)


void save_data_to_flash(){ 
    //save the data in different pages of the sector to reduce wear and tear
    uint32_t page = 0;
    bool found_page = false;
    for(;page < FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE; page++){
        uint32_t* addr = (uint32_t*)(XIP_BASE +  FLASH_LOCATION + (page * FLASH_PAGE_SIZE));
        if(*addr == 0xFFFFFFFF){
            found_page = true;
            break;
        }
    }

    uint32_t state = save_and_disable_interrupts();
    if(!found_page){
        flash_range_erase(FLASH_LOCATION, FLASH_SECTOR_SIZE);
        page = 0;
    }

    uint8_t data[FLASH_PAGE_SIZE];
    data[0] = program_state.is_celsius;
    data[1] = program_state.utc_offset;
    data[2] = program_state.is_celsius ^ program_state.utc_offset;
    data[3] = 0;
    flash_range_program(FLASH_LOCATION + page * FLASH_PAGE_SIZE, data, FLASH_PAGE_SIZE);
    restore_interrupts(state);
}


void get_data_from_flash(){
    //loop backwards from the end of the sector to find first location with valid data
    int32_t page = FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE - 1;
    for(;page > 0; page--){
        uint32_t* addr = (uint32_t*)(XIP_BASE +  FLASH_LOCATION + (page * FLASH_PAGE_SIZE));
        if(*addr != 0xFFFFFFFF){
            uint8_t* flash_addr = (uint8_t*)addr;
            uint8_t is_celsius = *flash_addr++;
            int8_t  utc_offset  = *flash_addr++;
            uint8_t check_sum  = *flash_addr;
            if((is_celsius ^ (uint8_t)utc_offset) == check_sum){
                program_state.is_celsius = is_celsius;
                program_state.utc_offset = utc_offset;
            }
            break;
        }
    }
}
