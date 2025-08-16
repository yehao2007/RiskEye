// 完整的光子订单引擎实现
module photonic_order_engine(
    input wire clk_156mhz,      // 156.25MHz系统时钟
    input wire reset_n,         // 异步复位
    input wire [127:0] order_data, // 订单数据
    input wire order_valid,     // 订单有效信号
    output reg photonic_pulse,  // 光子脉冲输出
    output reg order_ack,       // 订单确认
    output reg [31:0] latency   // 延迟统计(ns)
);

// 内部状态机和延迟测量逻辑
localparam IDLE = 2'b00;
localparam ENCODE = 2'b01;
localparam SEND = 2'b10;
localparam ACK = 2'b11;

reg [1:0] state;
reg [7:0] pulse_count;
reg [127:0] stored_order;
reg [31:0] start_time, end_time;

// 高精度计时器 (假设FPGA支持)
wire [31:0] current_time;
fpga_timer timer_inst(.clk(clk_156mhz), .time_out(current_time));

// 脉冲编码函数
function automatic reg encode_bit;
    input [127:0] data;
    input [7:0] bit_pos;
    begin
        // 1550nm激光脉冲宽度调制
        encode_bit = data[bit_pos];
    end
endfunction

// 状态机逻辑
always @(posedge clk_156mhz or negedge reset_n) begin
    if (!reset_n) begin
        state <= IDLE;
        photonic_pulse <= 1'b0;
        order_ack <= 1'b0;
        pulse_count <= 8'd0;
        latency <= 32'd0;
    end else begin
        case (state)
            IDLE:
                if (order_valid) begin
                    stored_order <= order_data;
                    state <= ENCODE;
                    pulse_count <= 8'd0;
                    start_time <= current_time; // 开始计时
                end
            ENCODE:
                begin
                    photonic_pulse <= encode_bit(stored_order, pulse_count);
                    pulse_count <= pulse_count + 8'd1;
                    if (pulse_count == 8'd127) begin
                        state <= SEND;
                        end_time <= current_time; // 结束计时
                        latency <= end_time - start_time; // 计算延迟
                    end
                end
            SEND:
                begin
                    photonic_pulse <= 1'b0;
                    state <= ACK;
                    order_ack <= 1'b1;
                end
            ACK:
                begin
                    order_ack <= 1'b0;
                    state <= IDLE;
                end
        endcase
    end
end

endmodule