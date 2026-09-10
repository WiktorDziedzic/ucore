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
logic [6:0]  avalon_address;
logic [31:0] avalon_writedata;
logic [3:0]  avalon_byteenable;
logic        avalon_readdatavalid;
logic        avalon_writeresponsevalid;
logic [31:0] avalon_readdata;
logic [1:0]  avalon_response;

logic        read_pending, read_pending_nxt;
logic        read_response_valid;
logic [31:0] read_response_data;

csr_eth__in_t  csr_hwif_in;
csr_eth__out_t csr_hwif_out;

logic          eth_tx_en_meta, eth_tx_en_meta_nxt;
logic          eth_tx_en_sync, eth_tx_en_sync_nxt;
logic          eth_tx_en_sync_d, eth_tx_en_sync_d_nxt;

logic          tx_busy_status, tx_busy_status_nxt;
logic          tx_seen_tx_en, tx_seen_tx_en_nxt;
logic          tx_start_request;

logic          ethernet_trigger;

logic          bypass;
logic [2:0]    loopback;

logic          tx_ready;
logic          tx_valid, tx_valid_nxt;
logic [1023:0] tx_data;
logic          tx_sop;
logic          tx_eop;
logic [7:0]    tx_empty;
logic [7:0]    tx_payload_len;
logic [31:0]   tx_frames_remaining, tx_frames_remaining_nxt;
logic [1023:0] tx_register_data;
logic          tx_frame_accepted;
logic          tx_frame_in_flight, tx_frame_in_flight_nxt;
logic          tx_end_of_frame;
logic          tx_configuration_valid;
logic          tx_active, tx_active_nxt;
logic [31:0]   tx_gap_counter, tx_gap_counter_nxt;

logic          rx_ready;
logic          rx_valid;
logic [1023:0] rx_data;
logic          rx_sop;
logic          rx_eop;
logic [7:0]    rx_empty;


/* Signals assignments */

assign ethernet_trigger = csr_hwif_out.ctrl.start.value;

assign tx_start_request = avalon_write && !avalon_waitrequest &&
                          avalon_address == 7'd0 &&
                          avalon_byteenable[0] && avalon_writedata[0];

assign bypass = 1'b0;

assign rx_ready = 1'b1;

assign tx_register_data = {
    csr_hwif_out.tx_data_31.value.value,
    csr_hwif_out.tx_data_30.value.value,
    csr_hwif_out.tx_data_29.value.value,
    csr_hwif_out.tx_data_28.value.value,
    csr_hwif_out.tx_data_27.value.value,
    csr_hwif_out.tx_data_26.value.value,
    csr_hwif_out.tx_data_25.value.value,
    csr_hwif_out.tx_data_24.value.value,
    csr_hwif_out.tx_data_23.value.value,
    csr_hwif_out.tx_data_22.value.value,
    csr_hwif_out.tx_data_21.value.value,
    csr_hwif_out.tx_data_20.value.value,
    csr_hwif_out.tx_data_19.value.value,
    csr_hwif_out.tx_data_18.value.value,
    csr_hwif_out.tx_data_17.value.value,
    csr_hwif_out.tx_data_16.value.value,
    csr_hwif_out.tx_data_15.value.value,
    csr_hwif_out.tx_data_14.value.value,
    csr_hwif_out.tx_data_13.value.value,
    csr_hwif_out.tx_data_12.value.value,
    csr_hwif_out.tx_data_11.value.value,
    csr_hwif_out.tx_data_10.value.value,
    csr_hwif_out.tx_data_9.value.value,
    csr_hwif_out.tx_data_8.value.value,
    csr_hwif_out.tx_data_7.value.value,
    csr_hwif_out.tx_data_6.value.value,
    csr_hwif_out.tx_data_5.value.value,
    csr_hwif_out.tx_data_4.value.value,
    csr_hwif_out.tx_data_3.value.value,
    csr_hwif_out.tx_data_2.value.value,
    csr_hwif_out.tx_data_1.value.value,
    csr_hwif_out.tx_data_0.value.value
};

assign tx_empty = 8'd128 - tx_payload_len;
assign tx_data = tx_register_data;
assign tx_sop = tx_valid;
assign tx_eop = tx_valid;
assign tx_frame_accepted = tx_valid && tx_ready;
assign tx_end_of_frame = tx_seen_tx_en &&
                         eth_tx_en_sync_d && !eth_tx_en_sync;
assign tx_configuration_valid = tx_payload_len != 8'd0 &&
                                csr_hwif_out.frame_count.value.value != 32'd0;

assign csr_hwif_in.status.busy.next = tx_busy_status_nxt;

assign loopback[0] = csr_hwif_out.loopback_ctrl.loopback_direct_lvl.value;
assign loopback[1] = csr_hwif_out.loopback_ctrl.loopback_analyzer_lvl.value;
assign loopback[2] = csr_hwif_out.loopback_ctrl.loopback_eth_ip_lvl.value;


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

    .hwif_in(csr_hwif_in),
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


/* Module internal logic */

always_comb begin
    if (csr_hwif_out.tx_length.value.value > 16'd128)
        tx_payload_len = 8'd128;
    else
        tx_payload_len = csr_hwif_out.tx_length.value.value[7:0];
end

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        tx_valid <= 1'b0;
        tx_active <= 1'b0;
        tx_frames_remaining <= 32'd0;
        tx_gap_counter <= 32'd0;
        tx_frame_in_flight <= 1'b0;
    end else begin
        tx_valid <= tx_valid_nxt;
        tx_active <= tx_active_nxt;
        tx_frames_remaining <= tx_frames_remaining_nxt;
        tx_gap_counter <= tx_gap_counter_nxt;
        tx_frame_in_flight <= tx_frame_in_flight_nxt;
    end
end

always_comb begin
    tx_valid_nxt = tx_valid;
    tx_active_nxt = tx_active;
    tx_frames_remaining_nxt = tx_frames_remaining;
    tx_gap_counter_nxt = tx_gap_counter;
    tx_frame_in_flight_nxt = tx_frame_in_flight;

    if (ethernet_trigger) begin
        tx_active_nxt = tx_configuration_valid;
        tx_frames_remaining_nxt = tx_configuration_valid ?
                                  csr_hwif_out.frame_count.value.value : 32'd0;
        tx_valid_nxt = 1'b0;
        tx_gap_counter_nxt = 32'd0;
        tx_frame_in_flight_nxt = 1'b0;
    end else begin
        if (tx_frame_accepted) begin
            tx_valid_nxt = 1'b0;
            tx_frame_in_flight_nxt = 1'b1;

            if (tx_frames_remaining != 32'd0)
                tx_frames_remaining_nxt = tx_frames_remaining - 32'd1;
        end else if (tx_end_of_frame && tx_frame_in_flight) begin
            tx_frame_in_flight_nxt = 1'b0;

            if (tx_frames_remaining == 32'd0) begin
                tx_active_nxt = 1'b0;
                tx_gap_counter_nxt = 32'd0;
            end else begin
                tx_gap_counter_nxt = csr_hwif_out.ifg_cycles.value.value;
            end
        end else if (!tx_valid && tx_active && !tx_frame_in_flight) begin
            if (tx_gap_counter != 32'd0) begin
                tx_gap_counter_nxt = tx_gap_counter - 32'd1;
            end else if (tx_frames_remaining != 32'd0) begin
                tx_valid_nxt = 1'b1;
            end
        end
    end
end

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        eth_tx_en_meta   <= 1'b0;
        eth_tx_en_sync   <= 1'b0;
        eth_tx_en_sync_d <= 1'b0;

        tx_busy_status   <= 1'b0;
        tx_seen_tx_en    <= 1'b0;
    end else begin
        eth_tx_en_meta   <= eth_tx_en_meta_nxt;
        eth_tx_en_sync   <= eth_tx_en_sync_nxt;
        eth_tx_en_sync_d <= eth_tx_en_sync_d_nxt;

        tx_busy_status   <= tx_busy_status_nxt;
        tx_seen_tx_en    <= tx_seen_tx_en_nxt;
    end
end

always_comb begin
    eth_tx_en_meta_nxt   = eth_tx_en;
    eth_tx_en_sync_nxt   = eth_tx_en_meta;
    eth_tx_en_sync_d_nxt = eth_tx_en_sync;

    tx_busy_status_nxt   = tx_busy_status;
    tx_seen_tx_en_nxt    = tx_seen_tx_en;

    if (tx_start_request || ethernet_trigger) begin
        tx_busy_status_nxt = tx_configuration_valid;
        tx_seen_tx_en_nxt  = 1'b0;
    end else begin
        if (eth_tx_en_sync) begin
            tx_seen_tx_en_nxt = 1'b1;
        end

        if (tx_busy_status && tx_end_of_frame) begin
            tx_seen_tx_en_nxt = 1'b0;

            if (tx_frames_remaining == 32'd0)
                tx_busy_status_nxt = 1'b0;
        end
    end
end

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        read_pending <= 1'b0;
        read_response_valid <= 1'b0;
        read_response_data <= 32'b0;
    end else begin
        read_pending <= read_pending_nxt;
        read_response_valid <= avalon_readdatavalid;

        if (avalon_readdatavalid)
            read_response_data <= avalon_readdata;
    end
end

always_comb begin
    local_addr = dbus.addr - ETHERNET_BASE_ADDRESS;

    avalon_address = local_addr[8:2];
    avalon_writedata = dbus.wdata;
    avalon_byteenable = dbus.be;

    avalon_read = 1'b0;
    avalon_write = 1'b0;

    dbus.stall = 1'b1;
    dbus.rvalid = 1'b0;
    dbus.rdata = read_response_data;

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
        dbus.stall = !read_response_valid;
        dbus.rvalid = read_response_valid;

        if (read_response_valid)
            read_pending_nxt = 1'b0;
    end
end

endmodule
