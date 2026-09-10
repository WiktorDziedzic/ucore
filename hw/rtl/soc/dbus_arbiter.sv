/* Copyright (C) 2025  AGH University of Krakow */

module dbus_arbiter
    import memory_map::*;
(
    input logic clk,
    input logic rst_n,

    dbus.slave  core_dbus,

    dbus.master code_rom_dbus,
    dbus.master data_ram_dbus,
    dbus.master gpio_dbus,
    dbus.master timer_dbus,
    dbus.master uart_dbus,
    dbus.master ethernet_dbus
);


/* User defined types */

typedef enum logic [2:0] {
    REQUESTS_PROCESSING,
    CODE_ROM_READOUT,
    DATA_RAM_READOUT,
    GPIO_READOUT,
    TIMER_READOUT,
    UART_READOUT,
    ETHERNET_READOUT
} state_t;


/* Local variables and signals */

state_t state, state_nxt;


/* Module internal logic */

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n)
        state <= REQUESTS_PROCESSING;
    else
        state <= state_nxt;
end

always_comb begin
    state_nxt = state;

    case (state)
    REQUESTS_PROCESSING: begin
        if (core_dbus.addr inside {[CODE_ROM_BASE_ADDRESS:CODE_ROM_END_ADDRESS]}) begin
            if (code_rom_dbus.rreq && !code_rom_dbus.stall)
                state_nxt = CODE_ROM_READOUT;
        end else if (core_dbus.addr inside {[DATA_RAM_BASE_ADDRESS:DATA_RAM_END_ADDRESS]}) begin
            if (data_ram_dbus.rreq && !data_ram_dbus.stall)
                state_nxt = DATA_RAM_READOUT;
        end else if (core_dbus.addr inside {[GPIO_BASE_ADDRESS:GPIO_END_ADDRESS]}) begin
            if (gpio_dbus.rreq)
                state_nxt = GPIO_READOUT;
        end else if (core_dbus.addr inside {[TIMER_BASE_ADDRESS:TIMER_END_ADDRESS]}) begin
            if (timer_dbus.rreq)
                state_nxt = TIMER_READOUT;
        end else if (core_dbus.addr inside {[UART_BASE_ADDRESS:UART_END_ADDRESS]}) begin
            if (uart_dbus.rreq)
                state_nxt = UART_READOUT;
        end else if (core_dbus.addr inside {[ETHERNET_BASE_ADDRESS:ETHERNET_END_ADDRESS]}) begin
            if (ethernet_dbus.rreq && !ethernet_dbus.stall)
                state_nxt = ETHERNET_READOUT;
        end
    end
    CODE_ROM_READOUT: begin
        state_nxt = REQUESTS_PROCESSING;
    end
    DATA_RAM_READOUT: begin
        state_nxt = REQUESTS_PROCESSING;
    end
    GPIO_READOUT: begin
        state_nxt = REQUESTS_PROCESSING;
    end
    TIMER_READOUT: begin
        state_nxt = REQUESTS_PROCESSING;
    end
    UART_READOUT: begin
        state_nxt = REQUESTS_PROCESSING;
    end
    ETHERNET_READOUT: begin
        if (ethernet_dbus.rvalid)
            state_nxt = REQUESTS_PROCESSING;
    end
    endcase
end

always_comb begin
    core_dbus.stall = 1'b1;
    core_dbus.rvalid = 1'b0;
    core_dbus.rdata = 32'b0;

    code_rom_dbus.addr = 32'b0;
    code_rom_dbus.be = 4'b0;
    code_rom_dbus.rreq = 1'b0;
    code_rom_dbus.wreq = 1'b0;
    code_rom_dbus.wdata = 32'b0;

    data_ram_dbus.addr = 32'b0;
    data_ram_dbus.be = 4'b0;
    data_ram_dbus.rreq = 1'b0;
    data_ram_dbus.wreq = 1'b0;
    data_ram_dbus.wdata = 32'b0;

    gpio_dbus.addr = 32'b0;
    gpio_dbus.be = 4'b0;
    gpio_dbus.rreq = 1'b0;
    gpio_dbus.wreq = 1'b0;
    gpio_dbus.wdata = 32'b0;

    timer_dbus.addr = 32'b0;
    timer_dbus.be = 4'b0;
    timer_dbus.rreq = 1'b0;
    timer_dbus.wreq = 1'b0;
    timer_dbus.wdata = 32'b0;

    uart_dbus.addr = 32'b0;
    uart_dbus.be = 4'b0;
    uart_dbus.rreq = 1'b0;
    uart_dbus.wreq = 1'b0;
    uart_dbus.wdata = 32'b0;

    ethernet_dbus.addr = 32'b0;
    ethernet_dbus.be = 4'b0;
    ethernet_dbus.rreq = 1'b0;
    ethernet_dbus.wreq = 1'b0;
    ethernet_dbus.wdata = 32'b0;

    case (state)
    REQUESTS_PROCESSING: begin
        if (core_dbus.addr inside {[CODE_ROM_BASE_ADDRESS:CODE_ROM_END_ADDRESS]}) begin
            core_dbus.stall = code_rom_dbus.stall;
            core_dbus.rvalid = code_rom_dbus.rvalid;
            core_dbus.rdata = code_rom_dbus.rdata;

            code_rom_dbus.addr = core_dbus.addr;
            code_rom_dbus.be = core_dbus.be;
            code_rom_dbus.rreq = core_dbus.rreq;
            code_rom_dbus.wreq = core_dbus.wreq;
            code_rom_dbus.wdata = core_dbus.wdata;
        end else if (core_dbus.addr inside {[DATA_RAM_BASE_ADDRESS:DATA_RAM_END_ADDRESS]}) begin
            core_dbus.stall = data_ram_dbus.stall;
            core_dbus.rvalid = data_ram_dbus.rvalid;
            core_dbus.rdata = data_ram_dbus.rdata;

            data_ram_dbus.addr = core_dbus.addr;
            data_ram_dbus.be = core_dbus.be;
            data_ram_dbus.rreq = core_dbus.rreq;
            data_ram_dbus.wreq = core_dbus.wreq;
            data_ram_dbus.wdata = core_dbus.wdata;
        end else if (core_dbus.addr inside {[GPIO_BASE_ADDRESS:GPIO_END_ADDRESS]}) begin
            core_dbus.stall = 1'b0;
            core_dbus.rvalid = 1'b1;
            core_dbus.rdata = gpio_dbus.rdata;

            gpio_dbus.addr = core_dbus.addr;
            gpio_dbus.be = core_dbus.be;
            gpio_dbus.rreq = core_dbus.rreq;
            gpio_dbus.wreq = core_dbus.wreq;
            gpio_dbus.wdata = core_dbus.wdata;
        end else if (core_dbus.addr inside {[TIMER_BASE_ADDRESS:TIMER_END_ADDRESS]}) begin
            core_dbus.stall = 1'b0;
            core_dbus.rvalid = 1'b1;
            core_dbus.rdata = timer_dbus.rdata;

            timer_dbus.addr = core_dbus.addr;
            timer_dbus.be = core_dbus.be;
            timer_dbus.rreq = core_dbus.rreq;
            timer_dbus.wreq = core_dbus.wreq;
            timer_dbus.wdata = core_dbus.wdata;
        end else if (core_dbus.addr inside {[UART_BASE_ADDRESS:UART_END_ADDRESS]}) begin
            core_dbus.stall = 1'b0;
            core_dbus.rvalid = 1'b1;
            core_dbus.rdata = uart_dbus.rdata;

            uart_dbus.addr = core_dbus.addr;
            uart_dbus.be = core_dbus.be;
            uart_dbus.rreq = core_dbus.rreq;
            uart_dbus.wreq = core_dbus.wreq;
            uart_dbus.wdata = core_dbus.wdata;
        end else if (core_dbus.addr inside {[ETHERNET_BASE_ADDRESS:ETHERNET_END_ADDRESS]}) begin
            core_dbus.stall = ethernet_dbus.stall;
            core_dbus.rvalid = ethernet_dbus.rvalid;
            core_dbus.rdata = ethernet_dbus.rdata;

            ethernet_dbus.addr = core_dbus.addr;
            ethernet_dbus.be = core_dbus.be;
            ethernet_dbus.rreq = core_dbus.rreq;
            ethernet_dbus.wreq = core_dbus.wreq;
            ethernet_dbus.wdata = core_dbus.wdata;
        end
    end
    CODE_ROM_READOUT: begin
        core_dbus.stall = code_rom_dbus.stall;
        core_dbus.rvalid = code_rom_dbus.rvalid;
        core_dbus.rdata = code_rom_dbus.rdata;

        code_rom_dbus.addr = core_dbus.addr;
        code_rom_dbus.be = core_dbus.be;
        code_rom_dbus.rreq = core_dbus.rreq;
        code_rom_dbus.wreq = core_dbus.wreq;
        code_rom_dbus.wdata = core_dbus.wdata;
    end
    DATA_RAM_READOUT: begin
        core_dbus.stall = data_ram_dbus.stall;
        core_dbus.rvalid = data_ram_dbus.rvalid;
        core_dbus.rdata = data_ram_dbus.rdata;

        data_ram_dbus.addr = core_dbus.addr;
        data_ram_dbus.be = core_dbus.be;
        data_ram_dbus.rreq = core_dbus.rreq;
        data_ram_dbus.wreq = core_dbus.wreq;
        data_ram_dbus.wdata = core_dbus.wdata;
    end
    GPIO_READOUT: begin
        core_dbus.stall = 1'b0;
        core_dbus.rvalid = 1'b1;
        core_dbus.rdata = gpio_dbus.rdata;

        gpio_dbus.addr = core_dbus.addr;
        gpio_dbus.be = core_dbus.be;
        gpio_dbus.rreq = core_dbus.rreq;
        gpio_dbus.wreq = core_dbus.wreq;
        gpio_dbus.wdata = core_dbus.wdata;
    end
    TIMER_READOUT: begin
        core_dbus.stall = 1'b0;
        core_dbus.rvalid = 1'b1;
        core_dbus.rdata = timer_dbus.rdata;

        timer_dbus.addr = core_dbus.addr;
        timer_dbus.be = core_dbus.be;
        timer_dbus.rreq = core_dbus.rreq;
        timer_dbus.wreq = core_dbus.wreq;
        timer_dbus.wdata = core_dbus.wdata;
    end
    UART_READOUT: begin
        core_dbus.stall = 1'b0;
        core_dbus.rvalid = 1'b1;
        core_dbus.rdata = uart_dbus.rdata;

        uart_dbus.addr = core_dbus.addr;
        uart_dbus.be = core_dbus.be;
        uart_dbus.rreq = core_dbus.rreq;
        uart_dbus.wreq = core_dbus.wreq;
        uart_dbus.wdata = core_dbus.wdata;
    end
    ETHERNET_READOUT: begin
        core_dbus.stall = ethernet_dbus.stall;
        core_dbus.rvalid = ethernet_dbus.rvalid;
        core_dbus.rdata = ethernet_dbus.rdata;

        ethernet_dbus.addr = core_dbus.addr;
        ethernet_dbus.be = core_dbus.be;
        ethernet_dbus.rreq = core_dbus.rreq;
        ethernet_dbus.wreq = core_dbus.wreq;
        ethernet_dbus.wdata = core_dbus.wdata;
    end
    endcase
end

endmodule
