module seg_time(
    input                   clk_50m    ,        // 时钟信号50MHZ
    input                   rst1  ,         // 复位信号1
	input                   rst2  ,         // 复位信号2
	input                   rst3 ,          // 复位信号3
	input                   rst4  ,         // 复位信号4
    input      wire   [7:0]     data   ,        // 4位数码管要显示的数值
    output   reg [1:0]     seg_sel,        // 数码管位选，最左侧数码管为最高位
    output   reg [6:0]     seg_led         // 数码管段选
    );

//parameter define
localparam  CLK_DIVIDE = 4'd10     ;        // 时钟分频系数
localparam  MAX_NUM    = 13'd5000  ;        // 对数码管驱动时钟(5MHz)计数1ms所需的计数值


reg    [3:0]             clk_cnt  ;        // 时钟分频计数器
reg                       dri_clk  ;        // 数码管的驱动时钟,5MHz
reg    [12:0]             cnt0     ;        // 数码管驱动时钟计数器
reg                       flag     ;        // 标志信号（标志着cnt0计数达1ms）
reg    [2:0]              cnt_sel  ;        // 数码管位选计数器
reg    [3:0]              num_disp ;        // 当前数码管显示的数据



//对系统时钟10分频，得到的频率为5MHz的数码管驱动时钟dri_clk
always @(posedge clk_50m or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4)
 begin
   if(!rst1||!rst2 || !rst3||!rst3||!rst4) begin
       clk_cnt <= 4'd0;
       dri_clk <= 1'b1;
   end
   else if(clk_cnt == CLK_DIVIDE/2 - 1'd1) begin
       clk_cnt <= 4'd0;
       dri_clk <= ~dri_clk;
   end
   else begin
       clk_cnt <= clk_cnt + 1'b1;
       dri_clk <= dri_clk;
   end
end


//每当计数器对数码管驱动时钟计数时间达1ms，输出一个时钟周期的脉冲信号
always @ (posedge dri_clk or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4) begin
    if (rst1 == 1'b0 ||rst2== 1'b0 ||rst3 == 1'b0 ||rst4 == 1'b0 ) begin
        cnt0 <= 13'b0;
        flag <= 1'b0;
     end
    else if (cnt0 < MAX_NUM - 1'b1) begin
        cnt0 <= cnt0 + 1'b1;
        flag <= 1'b0;
     end
    else begin
        cnt0 <= 13'b0;
        flag <= 1'b1;
     end
end

//cnt_sel从0计数到1，用于选择当前处于显示状态的数码管
always @ (posedge dri_clk or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4) begin
    if (rst1 == 1'b0 ||rst2== 1'b0 ||rst3 == 1'b0||rst4 == 1'b0 ) 
        cnt_sel <= 3'b0;
    else if(flag) begin
        if(cnt_sel < 3'd2)
            cnt_sel <= cnt_sel + 1'b1;
        else
            cnt_sel <= 3'b0;
      end
    else
        cnt_sel <= cnt_sel;
end

//控制数码管位选信号，使数码管轮流显示
always @ (posedge dri_clk or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4) begin
    if(!rst1||!rst2||!rst3||!rst3||!rst4) begin
        seg_sel  <= 4'b1111;              //位选信号低电平有效
        num_disp <= 4'b0;            
    end
    else begin
            case (cnt_sel)
                3'd0 :begin
                    seg_sel  <= 4'b1110;  //显示数码管最低位
                    num_disp <= data[3:0] ;  //显示的数据
                end
                3'd1 :begin
                    seg_sel  <= 4'b1101;  //显示数码管第1位
                    num_disp <= data[7:4] ;
                end
               
                default :begin
                    seg_sel  <= 4'b1111;
                    num_disp <= 4'b0;
                end
            endcase 
    end
end

//控制数码管段选信号，显示字符
always @ (posedge dri_clk or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4) begin
    if(!rst1||!rst2|| !rst3||!rst3||!rst4)
        seg_led <= 7'b1000000;
    else begin
        case (num_disp)
            4'd0 : seg_led <=  7'b0111111;  //显示数字 0
            4'd1 : seg_led <=  7'b0000110;  //显示数字 1
            4'd2 : seg_led <= 7'b1011011;   //显示数字 2
            4'd3 : seg_led <= 7'b1001111;   //显示数字 3
            4'd4 : seg_led <= 7'b1100110;   //显示数字 4
            4'd5 : seg_led <= 7'b1101101;   //显示数字 5
            4'd6 : seg_led <= 7'b1111101;   //显示数字 6
            4'd7 : seg_led <= 7'b0000111;   //显示数字 7
            4'd8 : seg_led <= 7'b1111111;   //显示数字 8
            4'd9 : seg_led <= 7'b1101111;   //显示数字 9
		  default: 
                   seg_led <= 8'b1111111;// 默认（可选）
        endcase
    end
end

endmodule 
