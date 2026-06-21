module ethernet
    import memory_map::*;
    import csr_eth_pkg::*;
(
    input  logic       clk,
    input  logic       rst_n,

    dbus.slave         dbus,

    output logic       eth_mdc,
    inout  logic       eth_mdio,
    output logic       eth_ref_clk,
    output logic       eth_rstn,

    input  logic       eth_tx_clk,
    output logic       eth_tx_en,
    output logic [3:0] eth_txd,

    input  logic       eth_rx_clk,
    input  logic       eth_rx_dv,
    input  logic [3:0] eth_rxd,

    input  logic       eth_col,
    input  logic       eth_crs,
    input  logic       eth_rxerr,

    output logic [3:0] eth_led
);


/* Local variables and signals */

logic [31:0] local_addr;

logic        avalon_read;
logic        avalon_write;
logic        avalon_waitrequest;
logic [3:0]  avalon_address;
logic [31:0] avalon_writedata;
logic [3:0]  avalon_byteenable;
logic        avalon_readdatavalid;
logic        avalon_writeresponsevalid;
logic [31:0] avalon_readdata;
logic [1:0]  avalon_response;

logic        read_pending, read_pending_nxt;

csr_eth__out_t csr_hwif_out;

logic          ethernet_trigger;

logic          bypass;
logic          loopback;

logic          tx_ready;
logic          tx_valid;
logic [1023:0] tx_data;
logic          tx_sop;
logic          tx_eop;
logic [7:0]    tx_empty;

logic          rx_ready;
logic          rx_valid;
logic [1023:0] rx_data;
logic          rx_sop;
logic          rx_eop;
logic [7:0]    rx_empty;


/* Signals assignments */

assign ethernet_trigger = csr_hwif_out.ctrl.trigger.value;

assign bypass = 1'b0;
assign loopback = 4'h0;

assign rx_ready = 1'b1;


/* Submodules placement */

csr_eth u_csr_eth (
    .clk(clk),
    .rst(!rst_n),

    .avalon_read,
    .avalon_write,
    .avalon_waitrequest,
    .avalon_address,
    .avalon_writedata,
    .avalon_byteenable,
    .avalon_readdatavalid,
    .avalon_writeresponsevalid,
    .avalon_readdata,
    .avalon_response,

    .hwif_out(csr_hwif_out)
);

ethernet_top u_ethernet_top (
    .clk_40Mhz(clk),
    .rst_n,

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

    .csr_hwif_out(csr_hwif_out),

    .ethernet_trigger,

    .bypass,
    .loopback,

    .tx_ready,
    .tx_valid,
    .tx_data,
    .tx_sop,
    .tx_eop,
    .tx_empty,

    .rx_ready,
    .rx_valid,
    .rx_data,
    .rx_sop,
    .rx_eop,
    .rx_empty,

    .led(eth_led)
);

data_generator u_data_generator (
    .clk_40Mhz(clk),
    .rst_n,


    .ethernet_trigger,

    .bypass,

    .ready(tx_ready),
    .valid(tx_valid),
    .data(tx_data),
    .sop(tx_sop),
    .eop(tx_eop),
    .empty(tx_empty)
);


/* Module internal logic */

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n)
        read_pending <= 1'b0;
    else
        read_pending <= read_pending_nxt;
end

always_comb begin
    local_addr = dbus.addr - ETHERNET_BASE_ADDRESS;

    avalon_address = local_addr[5:2];
    avalon_writedata = dbus.wdata;
    avalon_byteenable = dbus.be;

    avalon_read = 1'b0;
    avalon_write = 1'b0;

    dbus.stall = 1'b1;
    dbus.rvalid = 1'b0;
    dbus.rdata = avalon_readdata;

    read_pending_nxt = read_pending;

    if (!read_pending) begin
        avalon_read = dbus.rreq;
        avalon_write = dbus.wreq;

        dbus.stall = avalon_waitrequest;

        if (dbus.wreq && !avalon_waitrequest)
            dbus.rvalid = 1'b1;

        if (dbus.rreq && !avalon_waitrequest)
            read_pending_nxt = 1'b1;
    end else begin
        dbus.stall = !avalon_readdatavalid;
        dbus.rvalid = avalon_readdatavalid;

        if (avalon_readdatavalid)
            read_pending_nxt = 1'b0;
    end
end

endmodule