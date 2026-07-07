/* Copyright (C) 2025  AGH University of Krakow */

module soc (
    input logic         clk,
    input logic         rst_n,

    output logic        uart_sout,
    input logic         uart_sin,

    output logic [31:0] gpio_dout,
    input logic [31:0]  gpio_din,

    output logic        eth_mdc,
    inout  logic        eth_mdio,
    output logic        eth_ref_clk,
    output logic        eth_rstn,

    input  logic        eth_tx_clk,
    output logic        eth_tx_en,
    output logic [3:0]  eth_txd,

    input  logic        eth_rx_clk,
    input  logic        eth_rx_dv,
    input  logic [3:0]  eth_rxd,
    input  logic        eth_col,
    input  logic        eth_crs,
    input  logic        eth_rxerr,

    output logic [3:0]  eth_led
);


/* Local variables and signals */

ibus core_ibus ();

dbus core_dbus ();
dbus code_rom_dbus ();
dbus data_ram_dbus ();
dbus gpio_dbus ();
dbus timer_dbus ();
dbus uart_dbus ();
dbus ethernet_dbus ();


/* Submodules placement */

core u_core (
    .clk,
    .rst_n,

    .ibus(core_ibus),
    .dbus(core_dbus)
);

dbus_arbiter u_dbus_arbiter (
    .clk,
    .rst_n,

    .core_dbus,

    .code_rom_dbus,
    .data_ram_dbus,
    .gpio_dbus,
    .timer_dbus,
    .uart_dbus,
    .ethernet_dbus
);

code_rom u_code_rom (
    .clk,
    .rst_n,

    .ibus(core_ibus),
    .dbus(code_rom_dbus)
);

data_ram u_data_ram (
    .clk,
    .rst_n,

    .dbus(data_ram_dbus)
);

gpio u_gpio (
    .clk,
    .rst_n,

    .dbus(gpio_dbus),

    .dout(gpio_dout),
    .din(gpio_din)
);

timer u_timer (
    .clk,
    .rst_n,

    .dbus(timer_dbus)
);

uart u_uart (
    .clk,
    .rst_n,

    .dbus(uart_dbus),

    .sout(uart_sout),
    .sin(uart_sin)
);

ethernet u_ethernet (
    .clk,
    .rst_n,

    .dbus(ethernet_dbus),

    .eth_mdc,
    .eth_mdio,
    .eth_ref_clk,
    .eth_rstn,

    .eth_tx_clk,
    .eth_tx_en,
    .eth_txd,

    .eth_rx_clk,
    .eth_rx_dv,
    .eth_rxd,
    .eth_col,
    .eth_crs,
    .eth_rxerr,

    .eth_led
);

endmodule
