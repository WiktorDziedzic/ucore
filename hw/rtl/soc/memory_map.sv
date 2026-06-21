/* Copyright (C) 2025  AGH University of Krakow */

package memory_map;

const logic [31:0] CODE_ROM_BASE_ADDRESS = 32'h0000_0000,
                   CODE_ROM_END_ADDRESS = 32'h0000_3fff,

                   DATA_RAM_BASE_ADDRESS = 32'h1000_0000,
                   DATA_RAM_END_ADDRESS = 32'h1000_3fff,

                   GPIO_BASE_ADDRESS = 32'h2000_0000,
                   GPIO_END_ADDRESS = 32'h2000_0fff,

                   TIMER_BASE_ADDRESS = 32'h3000_0000,
                   TIMER_END_ADDRESS = 32'h3000_0fff,

                   UART_BASE_ADDRESS = 32'h4000_0000,
                   UART_END_ADDRESS = 32'h4000_0fff,

                   ETHERNET_BASE_ADDRESS = 32'h5000_0000,
                   ETHERNET_END_ADDRESS = 32'h5000_0fff;
endpackage
