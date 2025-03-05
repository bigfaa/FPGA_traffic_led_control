
`timescale 1ns / 1ps

module traffic_led_tb();

    // 声明测试平台中使用的信号，与被测试模块的端口类型和位宽对应
    reg clk_50m;
    reg rst1;
    reg rst2;
    reg rst3;
    reg rst4;
    wire [5:0] light1;
    wire [1:0] seg_sel;
    wire [6:0] seg_led;

    // 实例化被测试的 traffic_led 模块
    traffic_led uut (
       .clk_50m(clk_50m),
       .rst1(rst1),
       .rst2(rst2),
       .rst3(rst3),
       .rst4(rst4),
       .light1(light1),
       .seg_sel(seg_sel),
       .seg_led(seg_led)
    );

    // 产生时钟信号，这里生成 50MHz 的时钟（周期为 20ns）
    always #1 clk_50m = ~clk_50m;

    // 初始化测试过程
    initial begin
        // 初始化所有输入信号
        clk_50m = 1'b0;
        rst1 = 1'b1;
        rst2 = 1'b1;
        rst3 = 1'b1;
        rst4 = 1'b1;

        // 等待几个时钟周期，让系统稳定
        #100;

       

        // 正常运行一段时间，观察交通灯状态和数码管显示的变化
        #10000000;

        // 结束仿真
        $finish;
    end

endmodule